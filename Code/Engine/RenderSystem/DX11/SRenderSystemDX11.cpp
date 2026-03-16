/***************************************************************************
* SRenderSystemDX11.cpp
*/

#ifdef WIN32

#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SWindowsUtils.h"
#include "Core/SException.h"

#pragma comment(lib, "d3d11.lib")


SRenderSystemDX11::SRenderSystemDX11()
{
}

SRenderSystemDX11::~SRenderSystemDX11()
{
}

void SRenderSystemDX11::Create(void* windowHandle, const SAppFeaturesMap& features, SAppMode mode, const SAppContext& context)
{
	S_TRY

	HWND hWnd = (HWND)windowHandle;
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	auto VSyncIt = features.find(SAppFeature::VSync);
	bool bVSync = (VSyncIt != features.end() && VSyncIt->second.bValue());

	auto AllowFullscreenIt = features.find(SAppFeature::AllowFullscreen);
	bool bAllowFullscreen = (AllowFullscreenIt != features.end() && AllowFullscreenIt->second.bValue());

	// Find display mode
	DXGI_MODE_DESC displayModeDesc{};
	if (!SFindDisplayMode(width, height, &displayModeDesc))
	{
		throw std::exception("SRenderSystemDX11::Create(): Cannot find display mode");
	}

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc = displayModeDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = bVSync ? DXGI_SWAP_EFFECT_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = (mode == SAppMode::Windowed);
	swapChainDesc.Flags = bAllowFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	D3D_FEATURE_LEVEL featureLevel{};
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), device.GetAddressOf(), NULL, deviceContext.GetAddressOf())))
	{
		throw std::exception("SRenderSystemDX11::Create(): Cannot create device");
	}

	// Create back buffer and render target
	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
	{
		throw std::exception("SRenderSystemDX11::Create(): Cannot get back buffer");
	}

	if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf())))
	{
		throw std::exception("SRenderSystemDX11::Create(): Cannot create render target");
	}

	backBuffer.Reset();

	S_CATCH{ S_THROW("SRenderSystemDX11::Create()") }
}

void SRenderSystemDX11::Shutdown()
{
	renderTargetView.Reset();
	deviceContext.Reset();
	device.Reset();
	swapChain.Reset();
}

void SRenderSystemDX11::Update(float deltaSeconds, const SAppContext& context)
{
}

void SRenderSystemDX11::Render(const SAppContext& context)
{
}

void SRenderSystemDX11::Render(const class IVisual* visual, const SAppContext& context)
{
}

#endif // WIN32
