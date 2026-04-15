/***************************************************************************
* SWindowsApplication.cpp
*/

#ifdef WIN32

#include "Application/Windows/SWindowsApplication.h"
#include "Application/SApplicationModule.h"
#include "World/SWorldModule.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <windowsx.h>
#include <timeapi.h>
#include <thread>

#pragma comment (lib, "Winmm.lib")

#define WS_SQUARE_WINDOW_MENU   (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define WS_SQUARE_WINDOW         WS_POPUPWINDOW


static const WCHAR* SWindowClassName = L"SquareWindowClassName";
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

struct SWin32Handles
{
    SAppContext& appContext;
    IRenderSystemEx* renderEx;
    BOOL bTrackingMouse;
};


SWindowsApplication::SWindowsApplication()
{
    cmds.clear();
    cmdsCount = 0;
    windowSize = SSize2{ 800, 600 };
    appMode = SAppMode::Windowed;
    currentGameFrame = 0u;
    accumulatedFrames = 0;
    accumulatedTime = 0.0f;

    features[SAppFeature::VSync] = true;
    features[SAppFeature::NoDelay] = false;
    features[SAppFeature::HighFrequencyTimer] = false;
    features[SAppFeature::AllowResolutionChange] = false;
    features[SAppFeature::EnableFXAA] = false;
    features[SAppFeature::EnableHDR] = false;
    features[SAppFeature::ClearScreenColor] = SColor3(50, 50, 70);
    features[SAppFeature::ThreadPoolTasksPerThread] = static_cast<std::int32_t>(SConst::DefaultTasksPerThread);
    features[SAppFeature::ThreadPoolThreadsCount] = static_cast<std::int32_t>(SConst::DefaultThreadsInPool);
    features[SAppFeature::ThreadPoolDebugTrace] = false;
    features[SAppFeature::RenderSystemDebugTrace] = false;
}

SWindowsApplication::~SWindowsApplication()
{
    if (threadPool)
    {
        threadPool.reset();
    }

    if (world)
    {
        world.reset();
    }

    if (guiSystem)
    {
        guiSystem.reset();
    }

    if (localization)
    {
        localization.reset();
    }

    if (inputSystem)
    {
        inputSystem->Shutdown();
        inputSystem.reset();
    }

    if (renderSystem)
    {
        renderSystem->Shutdown();
        renderSystem.reset();
    }

    if (hWnd)
    {
        DestroyWindow(hWnd);
        UnregisterClassW(SWindowClassName, hInstance);
        hWnd = nullptr;
    }
}

void SWindowsApplication::Init(void* handle, const std::string& inCmds, std::int32_t inCmdsCount) noexcept
{
	hInstance = static_cast<HINSTANCE>(handle);
    cmds = inCmds;
    cmdsCount = inCmdsCount;
}

void SWindowsApplication::Init(void* handle, const std::string& inCmds) noexcept
{
    Init(handle, inCmds, 1);
}

void SWindowsApplication::Init(void* handle) noexcept
{
    Init(handle, "", 1);
}

void SWindowsApplication::Run()
{
    S_TRY

	if (!hInstance) throw std::exception("Invalid platform handle");
    using namespace std::chrono_literals;

    // create thread pool
    const std::int32_t tasksPerThread = GetFeatureValue(features, SAppFeature::ThreadPoolTasksPerThread);
    const std::int32_t threadsCount = GetFeatureValue(features, SAppFeature::ThreadPoolThreadsCount);
    const bool bEnableLogs = GetFeatureFlag(features, SAppFeature::ThreadPoolDebugTrace);
    threadPool = CreateThreadPool(tasksPerThread, threadsCount);
    threadPool->EnableDebugLogs(bEnableLogs);
    threadPool->Start();

    // set context
    context.app = this;
    context.pool = threadPool.get();
    context.render = renderSystem.get();

    // get window size
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BOOL windowedStyle = (windowSize.height < screenHeight) ? TRUE : FALSE;
    int style = windowedStyle ? WS_SQUARE_WINDOW_MENU : WS_SQUARE_WINDOW;

    RECT clientRect{ 0, 0, static_cast<LONG>(windowSize.width), static_cast<LONG>(windowSize.height) };
    AdjustWindowRect(&clientRect, style, FALSE);
    std::uint32_t winWidth = clientRect.right - clientRect.left;
    std::uint32_t winHeight = clientRect.bottom - clientRect.top;
    windowSize = SSize2{ winWidth, winHeight };

    // create window
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = SWindowClassName;
    if (0 == RegisterClassExW(&wcex))
    {
        throw std::exception("Cannot register window class");
    }

    hWnd = CreateWindowW(SWindowClassName, L"Game Window", style, CW_USEDEFAULT, CW_USEDEFAULT,
        winWidth, winHeight, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        throw std::exception("Cannot create window");
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    bHasFocus = true;

    // create game systems
    world = CreateWorld(context);
    context.world = world.get();

    inputSystem = SApplication::CreateDefaultInputSystem(context);
    context.input = inputSystem.get();

    localization = SApplication::CreateLocalization(context);
    context.text = localization.get();

    guiSystem = CreateGuiSystem(context);
    context.gui = guiSystem.get();

    if (renderSystem)
    {
        renderSystem->Subscribe(context);
        renderSystem->Create(hWnd, appMode, context);
        renderSystem->LoadShaders("../../Code/Shaders/HLSL/");
        context.render = renderSystem.get();
    }

    // set WndProc handles
    SWin32Handles handles{
        context, renderSystem.get(), FALSE
    };
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&handles));

    // set random
    SYSTEMTIME time;
    GetSystemTime(&time);
    srand(time.wMilliseconds + time.wSecond + time.wHour + time.wDay);

    // set loop variables
    startFrameTime = SClock::now();
    prevFrameTime = startFrameTime;
    currentGameFrame = 0u;
    accumulatedFrames = 0;
    accumulatedTime = 0.0f;

    const bool bNoDelay = GetFeatureFlag(features, SAppFeature::NoDelay);
    const bool bHighFrequencyTimer = GetFeatureFlag(features, SAppFeature::HighFrequencyTimer);

    // run game loop
    DebugMsg("[%s] SWindowsApplication::Run(): Start game loop\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str());
    if (bHighFrequencyTimer) timeBeginPeriod(1);

    // call app init handler
    if (initHandler)
    {
        S_TRY

        initHandler(context);

        S_CATCH{ S_THROW("SWindowsApplication::Run(onInitHandler)") }
    }

    MSG msg;
	while (!bQuit)
	{
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!bNoDelay)
        {
            std::this_thread::sleep_for(1ms);
        }

        // update and render
        OnUpdate();
	}

    DebugMsg("[%s] SWindowsApplication::Run(): End game loop\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str());
    if (bHighFrequencyTimer) timeEndPeriod(1);

    // set NULL to skip crash in WndProc
    SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

    S_CATCH {
        if (hWnd != NULL) SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
        S_THROW("SWindowsApplication::Run()")
    }
}

void SWindowsApplication::OnUpdate()
{
    S_TRY

    auto curTime = SClock::now();
    SDuration deltaSeconds = curTime - prevFrameTime;
    SDuration timeSeconds = curTime - startFrameTime;
    prevFrameTime = curTime;

    float deltaFloat = deltaSeconds.count();
    context.gameFrame = currentGameFrame;
    context.gameTime = timeSeconds.count();
    context.deltaSeconds = deltaFloat;

    if (updateHandler)
    {
        updateHandler(deltaFloat, context);
    }

    if (inputSystem)
    {
        inputSystem->Update(deltaFloat, context);
    }

    if (renderSystem)
    {
        renderSystem->Update(deltaFloat, context);

        if (renderSystem->CanRender())
        {
            renderSystem->Render(context);
        }
    }

    currentGameFrame++;
    accumulatedFrames++;
    accumulatedTime += deltaFloat;
    if (accumulatedTime >= 1.0f)
    {
        context.fps = accumulatedFrames;
        accumulatedFrames = 0;
        accumulatedTime = 0.0f;
    }

    S_CATCH{ S_THROW("SWindowsApplication::OnUpdate()") }
}

void SWindowsApplication::SetWindowSize(std::uint32_t width, std::uint32_t height, bool resizeRenderSystem)
{
    auto newSize = SSize2{ width, height };
    if (windowSize == newSize) return;

    windowSize = newSize;

    if (resizeRenderSystem && !windowSize.IsZero() &&
        renderSystem && renderSystem->CanRender())
    {
        // actual resize in WM_SIZE message handler
        renderSystem->RequestResize(width, height);
    }
}

std::any SWindowsApplication::GetFeature(SAppFeature feature) const noexcept
{
    auto featureIt = features.find(feature);
    return (featureIt != features.end()) ? featureIt->second : std::any();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SWin32Handles* handles = reinterpret_cast<SWin32Handles*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    SWindowsApplication* app = static_cast<SWindowsApplication*>(handles ? handles->appContext.app : nullptr);
    IGuiSystem* guiSystem = handles ? handles->appContext.gui : nullptr;
    IInputSystem* inputSystem = handles ? handles->appContext.input : nullptr;
    IInputDevice* activeKeyboard = nullptr;
    IInputDevice* activeMouse = nullptr;

    if (inputSystem)
    {
        const auto& devices = inputSystem->GetInputDevicesList();
        for (const auto& device : devices)
        {
            if (!device || !device->IsActive()) continue;
            if (device->GetType() == SInputDeviceType::Keyboard) activeKeyboard = device.get();
            if (device->GetType() == SInputDeviceType::Mouse) activeMouse = device.get();
        }
    }

    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    try
    {
        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_MENUCHAR:
            return MNC_CLOSE << 16; // disable exit fullscreen mode sound
        case WM_SIZE:
            if (handles)
            {
                WORD width = LOWORD(lParam);
                WORD height = HIWORD(lParam);

                if (handles->appContext.app)
                {
                    handles->appContext.app->SetWindowSize(width, height, false);
                }

                if (handles->renderEx &&
                    width >= 640 && height >= 480)
                {
                    handles->renderEx->Resize(SSize2{ width, height }, handles->appContext);
                }
            }
            break;
        case WM_SETFOCUS:
            if (app) app->SetFocusState(true);
            break;
        case WM_KILLFOCUS:
            if (app) app->SetFocusState(false);
            break;
        case WM_KEYDOWN:
            if (activeKeyboard) activeKeyboard->SetState(static_cast<std::int32_t>(wParam), true);
            if (guiSystem) guiSystem->OnKeys(static_cast<std::int32_t>(wParam), SKeyState::Down, handles->appContext);
            break;
        case WM_KEYUP:
            if (activeKeyboard) activeKeyboard->SetState(static_cast<std::int32_t>(wParam), false);
            if (guiSystem) guiSystem->OnKeys(static_cast<std::int32_t>(wParam), SKeyState::Up, handles->appContext);
            break;
        case WM_MOUSEMOVE:
            if (guiSystem) guiSystem->OnMouseMove(x, y, handles->appContext);
            if (handles && !handles->bTrackingMouse)
            {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                if (TrackMouseEvent(&tme))
                {
                    handles->bTrackingMouse = TRUE;
                }
            }
            break;
        case WM_MOUSELEAVE:
            if (guiSystem) guiSystem->OnMouseLeave();
            if (handles) handles->bTrackingMouse = FALSE;
            break;
        case WM_LBUTTONDOWN:
            if (guiSystem) guiSystem->OnMouseButton(SMouseBtn::Left, SKeyState::Down, x, y, handles->appContext);
            break;
        case WM_LBUTTONUP:
            if (guiSystem) guiSystem->OnMouseButton(SMouseBtn::Left, SKeyState::Up, x, y, handles->appContext);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    catch (const std::exception& ex)
    {
        DebugMsg("SWindowsApplication::WndProc()>\n%s\n", ex.what());
        if (hWnd) DestroyWindow(hWnd);
    }

    return 0;
}

#endif // WIN32
