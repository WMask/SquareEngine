/***************************************************************************
* SRenderSystemDX11.cpp
*/

#ifdef WIN32

#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SWindowsUtils.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#pragma comment(lib, "d3d11.lib")


SRenderSystemDX11::SRenderSystemDX11()
{
}

SRenderSystemDX11::~SRenderSystemDX11()
{
}

void SRenderSystemDX11::Create(void* windowHandle, const SAppFeaturesMap& inFeatures, SAppMode mode, const SAppContext& context)
{
	S_TRY

	features = inFeatures;
	HWND hWnd = static_cast<HWND>(windowHandle);
	HDC hDC = GetDC(hWnd);
	int maxRefreshRate = GetDeviceCaps(hDC, VREFRESH);
	ReleaseDC(hWnd, hDC);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	bool bAllowFullscreen = GetFeatureFlag(features, SAppFeature::AllowFullscreen);

	// Find display mode
	DXGI_MODE_DESC displayModeDesc{};
	if (!SFindDisplayMode(width, height, maxRefreshRate, &displayModeDesc))
	{
		throw std::exception("Cannot find display mode");
	}

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
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
		D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), d3dDevice.GetAddressOf(), NULL, deviceContext.GetAddressOf())))
	{
		throw std::exception("Cannot create device");
	}

	// Create back buffer and render target
	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
	{
		throw std::exception("Cannot get back buffer");
	}

	if (FAILED(d3dDevice->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf())))
	{
		throw std::exception("Cannot create render target");
	}

	backBuffer.Reset();

	DebugMsg("SRenderSystemDX11::Create(): Render system created\n");

	S_CATCH{ S_THROW("SRenderSystemDX11::Create()") }
}

void SRenderSystemDX11::Shutdown()
{
	renderTargetView.Reset();
	deviceContext.Reset();
	d3dDevice.Reset();
	swapChain.Reset();
}

void SRenderSystemDX11::Update(float deltaSeconds, const SAppContext& context)
{
}

void SRenderSystemDX11::Render(const SAppContext& context)
{
	S_TRY

	if (!deviceContext || !swapChain)
	{
		throw std::exception("Invalid render device");
	}

	auto ClearColorIt = features.find(SAppFeature::ClearScreenColor);
	if (ClearColorIt->second.has_value())
	{
		SColor3 color3 = std::any_cast<SColor3>(ClearColorIt->second);
		SColor4F color4 = SConvert::FromSColor3(color3);
		deviceContext->ClearRenderTargetView(renderTargetView.Get(), color4);
	}

	bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	swapChain->Present(bVSync ? DXGI_SWAP_EFFECT_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD, 0);

	S_CATCH{ S_THROW("SRenderSystemDX11::Render()") }
}

void SRenderSystemDX11::Render(const class IVisual* visual, const SAppContext& context)
{
}

#endif // WIN32
