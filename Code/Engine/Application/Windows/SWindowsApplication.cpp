/***************************************************************************
* SWindowsApplication.cpp
*/

#ifdef WIN32

#include "Application/Windows/SWindowsApplication.h"
#include "Application/SApplicationModule.h"
#include "Core/SException.h"

#include <timeapi.h>
#include <windowsx.h>

#pragma comment (lib, "Winmm.lib")

#define WS_SQUARE_WINDOW_MENU   (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define WS_SQUARE_WINDOW         WS_POPUPWINDOW


static const WCHAR* SWindowClassName = L"SquareWindowClassName";
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

struct SWin32Handles
{
    SAppContext& appContext;
    IApplication& app;
};


SWindowsApplication::SWindowsApplication()
{
    cmds.clear();
    cmdsCount = 0;
    windowSize = SSize2{ 800, 600 };
    appMode = SAppMode::Windowed;
    prevTime.QuadPart = 0;
    frequency.QuadPart = 0;
    features[SAppFeature::VSync] = true;
}

SWindowsApplication::~SWindowsApplication()
{
    if (hWnd)
    {
        DestroyWindow(hWnd);
        UnregisterClassW(SWindowClassName, hInstance);
        hWnd = nullptr;
    }
}

void SWindowsApplication::Init(void* handle, const std::string& inCmds, int inCmdsCount) noexcept
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

	if (!hInstance) throw std::exception("SWindowsApplication::Run(): Invalid platform handle");

    // set context
    context.app = this;

    // get window size
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BOOL windowedStyle = (windowSize.height < screenHeight) ? TRUE : FALSE;
    int style = windowedStyle ? WS_SQUARE_WINDOW_MENU : WS_SQUARE_WINDOW;

    RECT clientRect{ 0, 0, windowSize.width, windowSize.height };
    AdjustWindowRect(&clientRect, style, FALSE);
    int winWidth = clientRect.right - clientRect.left;
    int winHeight = clientRect.bottom - clientRect.top;

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
        throw std::exception("SWindowsApplication::Run(): Cannot register window class");
    }

    hWnd = CreateWindowW(SWindowClassName, L"Game Window", style, CW_USEDEFAULT, CW_USEDEFAULT,
        winWidth, winHeight, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        throw std::exception("SWindowsApplication::Run(): Cannot create window");
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    context.windowHandle = hWnd;

    SYSTEMTIME time;
    GetSystemTime(&time);
    srand(time.wMilliseconds);

    // set handles
    SWin32Handles handles {
        context, *this
    };
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&handles));

    // call app init handler
    if (initHandler)
    {
        S_TRY

        initHandler(context);

        S_CATCH{ S_THROW("SWindowsApplication::Run(onInitHandler)") }
    }

    // run game loop
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prevTime);

    const bool noDelay = features[SAppFeature::NoDelay].bValue();
    MSG msg;
	while (!quit)
	{
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (!noDelay)
            {
                timeBeginPeriod(1);
                Sleep(1);
                timeEndPeriod(1);
            }

            // update and render
            OnUpdate();
        }
	}

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

    LARGE_INTEGER curTime, deltaTime;
    QueryPerformanceCounter(&curTime);
    deltaTime.QuadPart = (curTime.QuadPart - prevTime.QuadPart);

    if (curTime.QuadPart != prevTime.QuadPart)
    {
        prevTime.QuadPart = curTime.QuadPart;

        double deltaSeconds = static_cast<double>(deltaTime.QuadPart) / static_cast<double>(frequency.QuadPart);
        float deltaFloat = static_cast<float>(deltaSeconds);

        double timeSeconds = static_cast<double>(curTime.QuadPart) / static_cast<double>(frequency.QuadPart);
        context.gameTime = static_cast<float>(timeSeconds);

        if (updateHandler)
        {
            updateHandler(deltaFloat, context);
        }
    }

    S_CATCH{ S_THROW("SWindowsApplication::OnUpdate()") }
}

void SWindowsApplication::SetWindowSize(std::int32_t width, std::int32_t height)
{
    auto newSize = SSize2{ width, height };
    if (windowSize == newSize) return;

    windowSize = newSize;
}

void SWindowsApplication::SetWindowMode(SAppMode mode)
{
    appMode = mode;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SWin32Handles* handles = reinterpret_cast<SWin32Handles*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    S_TRY

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
        break;
    case WM_KEYDOWN:
        break;
    case WM_KEYUP:
        break;
    case WM_MOUSEMOVE:
        break;
    case WM_LBUTTONDOWN:
        break;
    case WM_LBUTTONUP:
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    S_CATCH{ S_THROW("SWindowsApplication::WndProc()") }

    return 0;
}

#endif // WIN32
