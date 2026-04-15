/***************************************************************************
* SRenderSystemDX11.cpp
*/

#ifdef WIN32

#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/Windows/SUtilsWindows.h"
#include "RenderSystem/SECSComponents.h"
#include "Application/SApplicationInterface.h"
#include "Core/SException.h"
#include "Core/SUtils.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include <dxgi1_6.h>

#pragma comment(lib, "d3d11.lib")


SRenderSystemDX11::SRenderSystemDX11()
	: coloredSpriteRender(*this)
	, texturedSpriteRender(*this)
	, frameAnimSpriteRender(*this)
	, textRender(*this)
	, meshRender(*this)
	, fxaaRender(*this)
{
}

SRenderSystemDX11::~SRenderSystemDX11()
{
	Shutdown();
}

void SRenderSystemDX11::Create(void* windowHandle, SAppMode mode, const SAppContext& context)
{
	S_TRY

	auto& features = context.app->GetFeatures();
	world = context.world;
	appMode = mode;

	hWnd = static_cast<HWND>(windowHandle);
	HDC hDC = GetDC(hWnd);
	cachedMaxRefreshRate = GetDeviceCaps(hDC, VREFRESH);
	int ScreenW = GetDeviceCaps(hDC, HORZRES);
	int ScreenH = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(hWnd, hDC);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	std::uint32_t width = clientRect.right - clientRect.left;
	std::uint32_t height = clientRect.bottom - clientRect.top;
	SSize2 viewportSize{ width, height };

	bAllowFXAA = GetFeatureFlag(features, SAppFeature::EnableFXAA) ? TRUE : FALSE;
	const bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	const bool bShouldGoFullscreen = (ScreenW == width && ScreenH == height);
	if (bShouldGoFullscreen)
	{
		// use fullscreen mode if window size matching
		appMode = SAppMode::Fullscreen;
	}

	// allow force monitor resolution change
	const bool bAllowResolutionChange = GetFeatureFlag(features, SAppFeature::AllowResolutionChange);
	const bool bWantFullscreenButWindowNotMatching = appMode == SAppMode::Fullscreen && !bShouldGoFullscreen;
	if (bWantFullscreenButWindowNotMatching && !bAllowResolutionChange)
	{
		throw std::exception("Cannot go to fullscreen mode without SAppFeature::AllowResolutionChange");
	}

	// get hardware adapter
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(dxGIFactory.GetAddressOf()))))
	{
		throw std::exception("Cannot create DX factory");
	}

	ComPtr<IDXGIAdapter1> dxGIAdapter;
	if (!GetHardwareAdapter(dxGIFactory, dxGIAdapter))
	{
		throw std::exception("Cannot get hardware adapter");
	}

	// check features
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(dxGIFactory.As(&factory4)))
	{
		bFlipPresent = TRUE;
	}

	ComPtr<IDXGIFactory5> factory5;
	if (SUCCEEDED(dxGIFactory.As(&factory5)))
	{
		const bool bEnableHDR = GetFeatureFlag(features, SAppFeature::EnableHDR);
		if (bEnableHDR) bAllowHDR = IsDisplayHDR10(dxGIFactory.Get()) ? TRUE : FALSE;

		factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&bAllowTearing, sizeof(bAllowTearing));
	}

	swapChainFlags = 0u;
	if (bAllowResolutionChange) swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (bAllowTearing && !bShouldGoFullscreen) swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	const UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	backBufferFormat = bAllowHDR ? SConst::DefaultHDRBackBufferFormat : SConst::DefaultBackBufferFormat;

	// create device
	D3D_FEATURE_LEVEL createdLevel{};
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1
	};

	ComPtr<ID3D11Device> newDevice;
	ComPtr<ID3D11DeviceContext> newDeviceContext;
	if (FAILED(D3D11CreateDevice(dxGIAdapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags,
		featureLevels, ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, newDevice.GetAddressOf(),
		&createdLevel, newDeviceContext.GetAddressOf())))
	{
		bAllowHDR = FALSE;

		featureLevels[0] = D3D_FEATURE_LEVEL_11_0;
		if (FAILED(D3D11CreateDevice(dxGIAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags,
			featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, newDevice.GetAddressOf(),
			&createdLevel, newDeviceContext.GetAddressOf())))
		{
			throw std::exception("Cannot create device");
		}
	}

	DebugMsg("[%s] SRenderSystemDX11::Create(): D3D_11.%d device, AllowTearing=%d, AllowHDR=%d, FlipPresent=%d\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str(),
		(createdLevel == D3D_FEATURE_LEVEL_11_1) ? 1 : 0, bAllowTearing, bAllowHDR, bFlipPresent);

	ComPtr<ID3D11Device5> d3dDevice5;
	if (SUCCEEDED(newDevice.As(&d3dDevice5)))
	{
		d3dDevice = d3dDevice5;
	}
	else
	{
		d3dDevice = newDevice;
	}

	ComPtr<ID3D11DeviceContext4> deviceContext4;
	if (SUCCEEDED(newDeviceContext.As(&deviceContext4)))
	{
		deviceContext = deviceContext4;
	}
	else
	{
		deviceContext = newDeviceContext;
	}

	// create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferCount = SConst::DefaultBackBufferCount;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = swapChainFlags;
	swapChainDesc.SampleDesc.Count = 1;

	std::string swapEffectName;
	D3D11_FEATURE_DATA_D3D11_OPTIONS3 options3{};
	HRESULT hFeatureGetResult = d3dDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options3, sizeof(options3));
	if (SUCCEEDED(hFeatureGetResult) && options3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer)
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapEffectName = "SWAP_EFFECT_FLIP_DISCARD";
	}
	else
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapEffectName = "SWAP_EFFECT_FLIP_SEQUENTIAL";
	}

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFsDesc{};
	swapChainFsDesc.Windowed = (appMode == SAppMode::Windowed) ? TRUE : FALSE;

	ComPtr<IDXGISwapChain1> newSwapChain;
	if (FAILED(dxGIFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hWnd,
		&swapChainDesc, &swapChainFsDesc, NULL, newSwapChain.GetAddressOf())))
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapEffectName = "SWAP_EFFECT_DISCARD";

		if (FAILED(dxGIFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hWnd,
			&swapChainDesc, NULL, NULL, newSwapChain.GetAddressOf())))
		{
			throw std::exception("Cannot create swap chain");
		}
	}

	DebugMsg("[%s] SRenderSystemDX11::Create(): Created swapchain type: %s\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str(), swapEffectName.c_str());

	newSwapChain.As(&swapChain);

	if (FAILED(dxGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
	{
		throw std::exception("Cannot make window association");
	}

	UpdateColorSpace();
	CreateRenderTargetAndDepthStencil(width, height);

	if (bAllowFXAA)
	{
		sdrScene.Init(d3dDevice.Get(), SConst::DefaultSDRRenderTargetFormat);
		sdrScene.Create(viewportSize);
	}

	if (bAllowHDR)
	{
		hdrScene.Init(d3dDevice.Get(), SConst::DefaultHDRRenderTargetFormat);
		hdrScene.Create(viewportSize);
	}

	// set up rasterizer
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthClipEnable = TRUE;

	if (FAILED(d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf())))
	{
		throw std::exception("Cannot create rasterizer state");
	}

	deviceContext->RSSetState(rasterizerState.Get());

	// create blend state
	D3D11_BLEND_DESC blendStateDesc{};
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	d3dDevice->CreateBlendState(&blendStateDesc, blendState.GetAddressOf());
	deviceContext->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

	// create samplers
	D3D11_SAMPLER_DESC desc{};
	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MaxLOD = FLT_MAX;

	if (FAILED(d3dDevice->CreateSamplerState(&desc, surfaceSampler.GetAddressOf())))
	{
		throw std::exception("Cannot create surface sampler");
	}

	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	if (FAILED(d3dDevice->CreateSamplerState(&desc, cubemapSampler.GetAddressOf())))
	{
		throw std::exception("Cannot create cubemap sampler");
	}

	desc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	if (FAILED(d3dDevice->CreateSamplerState(&desc, pointSampler.GetAddressOf())))
	{
		throw std::exception("Cannot create render target sampler");
	}

	ID3D11SamplerState* samplers[] = {
		surfaceSampler.Get(), cubemapSampler.Get(), pointSampler.Get()
	};
	deviceContext->PSSetSamplers(0, 3, samplers);

	// init managers
	shaderManager.Init(context.pool);
	textureManager.Init(context.pool, this);
	meshManager.Init(context.pool, this);

	cachedCameraPos2d = SVector3{ width / 2.0f, height / 2.0f, 1.0f };
	cachedCameraTarget2d = SVector3{ cachedCameraPos2d.x, cachedCameraPos2d.y, 0.0f };
	context.world->GetCamera().Set(SCameraSpace::Camera2D, cachedCameraPos2d, cachedCameraTarget2d);

	cachedCameraPos3d = SVector3{ 500.0f, 500.0f, 500.0f };
	cachedCameraTarget3d = SConst::ZeroSVector3;
	context.world->GetCamera().Set(SCameraSpace::Camera3D, cachedCameraPos3d, cachedCameraTarget3d);

	constantBuffers.Init(d3dDevice.Get(), deviceContext.Get(), context.world->GetCamera(), width, height);
	context.world->UpdateWorldScale(viewportSize);
	cachedRenderSystemSize = SSize2{ width, height };
	cachedWorld2dScale = context.world->GetScale().GetScale();

	SetMode(appMode);

	bCachedNeedDebugTrace = GetFeatureFlag(features, SAppFeature::RenderSystemDebugTrace);
	DebugMsg("[%s] SRenderSystemDX11::Create(): Render system created\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str());

	S_CATCH{ S_THROW("SRenderSystemDX11::Create()") }
}

bool SRenderSystemDX11::IsDisplayHDR10(IDXGIFactory2* factory)
{
	RECT windowBounds;
	if (!GetWindowRect(hWnd, &windowBounds))
	{
		return false;
	}

	const long ax1 = windowBounds.left;
	const long ay1 = windowBounds.top;
	const long ax2 = windowBounds.right;
	const long ay2 = windowBounds.bottom;

	ComPtr<IDXGIOutput> bestOutput;
	long bestIntersectArea = -1;

	ComPtr<IDXGIAdapter> adapter;
	for (UINT adapterIndex = 0;
		SUCCEEDED(factory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
		++adapterIndex)
	{
		ComPtr<IDXGIOutput> output;
		for (UINT outputIndex = 0;
			SUCCEEDED(adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
			++outputIndex)
		{
			DXGI_OUTPUT_DESC desc;
			if (FAILED(output->GetDesc(&desc)))
			{
				return false;
			}

			const auto& rect = desc.DesktopCoordinates;
			const long intersectArea = SMath::ComputeIntersectionArea(ax1, ay1, ax2, ay2,
				rect.left, rect.top, rect.right, rect.bottom);
			if (intersectArea > bestIntersectArea)
			{
				bestOutput.Swap(output);
				bestIntersectArea = intersectArea;
			}
		}
	}

	if (bestOutput)
	{
		ComPtr<IDXGIOutput6> output6;
		if (SUCCEEDED(bestOutput.As(&output6)))
		{
			DXGI_OUTPUT_DESC1 desc;
			if (FAILED(output6->GetDesc1(&desc)))
			{
				return false;
			}

			if (desc.ColorSpace == SConst::DefaultHDRColorSpace)
			{
				return true;
			}
		}
	}

	return false;
}

void SRenderSystemDX11::UpdateColorSpace()
{
	if (!dxGIFactory) return;

	S_TRY

	if (!dxGIFactory->IsCurrent())
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(dxGIFactory.GetAddressOf()))))
		{
			throw std::exception("Cannot create DX factory");
		}
	}

	colorSpace = SConst::DefaultSDRColorSpace;
	bool bIsDisplayHDR10 = IsDisplayHDR10(dxGIFactory.Get());
	if (bIsDisplayHDR10)
	{
		switch (backBufferFormat)
		{
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			// The application creates the HDR10 signal
			colorSpace = SConst::DefaultHDRColorSpace;
			break;
		default:
			break;
		}
	}

	ComPtr<IDXGISwapChain3> swapChain3;
	if (bAllowHDR && swapChain && SUCCEEDED(swapChain.As(&swapChain3)))
	{
		UINT colorSpaceSupport = 0;
		if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
			&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
		{
			if (FAILED(swapChain3->SetColorSpace1(colorSpace)))
			{
				throw std::exception("Cannot set color space");
			}
			else
			{
				DebugMsg("[%s] SRenderSystemDX11::UpdateColorSpace(): Color space changed to HDR\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str());
			}
		}
	}

	S_CATCH{ S_THROW("SRenderSystemDX11::UpdateColorSpace()") }
}

void SRenderSystemDX11::CreateRenderTargetAndDepthStencil(std::uint32_t width, std::uint32_t height)
{
	S_TRY

	if (!swapChain || !d3dDevice || !deviceContext)
	{
		throw std::exception("Invalid parameters");
	}

	// create render target
	if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(renderTarget.GetAddressOf()))))
	{
		throw std::exception("Cannot get render target");
	}

	if (FAILED(d3dDevice->CreateRenderTargetView(renderTarget.Get(), NULL, renderTargetView.GetAddressOf())))
	{
		throw std::exception("Cannot create render target");
	}

	// set depth stencil view
	D3D11_TEXTURE2D_DESC depthBufferDesc{};
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(d3dDevice->CreateTexture2D(&depthBufferDesc, NULL, depthStencil.GetAddressOf())))
	{
		throw std::exception("Cannot create depth stencil buffer");
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	if (FAILED(d3dDevice->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf())))
	{
		throw std::exception("Cannot create depth stencil state");
	}

	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if (FAILED(d3dDevice->CreateDepthStencilView(depthStencil.Get(),
		&depthStencilViewDesc, depthStencilView.GetAddressOf())))
	{
		throw std::exception("Cannot create depth stencil view");
	}

	// set the viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	deviceContext->RSSetViewports(1, &viewPort);

	S_CATCH{ S_THROW("SRenderSystemDX11::CreateRenderTargetViewAndSwapChain()") }
}

void SRenderSystemDX11::Shutdown()
{
	fxaaRender.Shutdown();
	meshRender.Shutdown();
	textRender.Shutdown();
	frameAnimSpriteRender.Shutdown();
	texturedSpriteRender.Shutdown();
	coloredSpriteRender.Shutdown();
	constantBuffers.Shutdown();
	meshManager.Shutdown();
	textureManager.Shutdown();
	shaderManager.Shutdown();
	sdrScene.Shutdown();
	hdrScene.Shutdown();
	cubemapMIPLevels.clear();
	rasterizerState.Reset();
	surfaceSampler.Reset();
	cubemapSampler.Reset();
	pointSampler.Reset();
	blendState.Reset();
	renderTarget.Reset();
	depthStencil.Reset();
	renderTargetView.Reset();
	depthStencilView.Reset();
	depthStencilState.Reset();
	deviceContext.Reset();
	swapChain.Reset();
	d3dDevice.Reset();
	dxGIFactory.Reset();
}

void SRenderSystemDX11::LoadShaders(const std::filesystem::path& folderPath)
{
	S_TRY

	if (!d3dDevice)
	{
		throw std::exception("Invalid render device");
	}

	// collect shader paths
	std::vector<std::filesystem::path> paths;
	for (auto& entry : std::filesystem::directory_iterator(folderPath))
	{
		if (entry.is_regular_file())
		{
			paths.push_back(entry.path());
		}
	}

	// request load & compile
	shaderManager.LoadShaders(paths, [this](const SShaderManagerWindows::SCompiledShader& shaderData)
	{
		if (!d3dDevice)
		{
			throw std::exception("Invalid render device");
		}

		SShaderDataDX11 shader;

		if (FAILED(d3dDevice->CreateVertexShader(shaderData.vsCode->GetBufferPointer(),
			shaderData.vsCode->GetBufferSize(), NULL, shader.vs.GetAddressOf())))
		{
			throw std::exception("Cannot create vertex shader");
		}

		if (FAILED(d3dDevice->CreatePixelShader(shaderData.psCode->GetBufferPointer(),
			shaderData.psCode->GetBufferSize(), NULL, shader.ps.GetAddressOf())))
		{
			throw std::exception("Cannot create vertex shader");
		}

		shader.vsCode = shaderData.vsCode.Get();
		if (coloredSpriteRender.CheckShaderName(shaderData.name))
		{
			coloredSpriteRender.Setup(shader);
		}
		else if (texturedSpriteRender.CheckShaderName(shaderData.name))
		{
			texturedSpriteRender.Setup(shader);
			frameAnimSpriteRender.Setup(shader);
			frameAnimSpriteRender.CheckShaderName(shaderData.name);
		}
		else if (textRender.CheckShaderName(shaderData.name))
		{
			textRender.Setup(shader);
		}
		else if (meshRender.CheckShaderName(shaderData.name))
		{
			meshRender.Setup(shader);
		}
		else if (fxaaRender.CheckShaderName(shaderData.name))
		{
			fxaaRender.Setup(shader);
		}

		shaders.emplace(shaderData.name, shader);
	});

	S_CATCH{ S_THROW("SRenderSystemDX11::LoadShaders()") }
}

STexID SRenderSystemDX11::LoadTexture(const std::filesystem::path& texturePath)
{
	return textureManager.LoadTexture(texturePath);
}

void SRenderSystemDX11::PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate)
{
	textureManager.PreloadTextures(paths, delegate);
}

std::pair<SSize2, bool> SRenderSystemDX11::GetTextureSize(STexID id) const
{
	auto texture = static_cast<STextureDataDX11*>(textureManager.FindTexture(id));
	if (texture)
	{
		return { texture->texSize, true };
	}

	return { SConst::ZeroSSize2, false };
}

void SRenderSystemDX11::LoadCubemap(const std::filesystem::path& path, ECubemapType type)
{
	textureManager.LoadCubemap(path, type);
	constantBuffers.UpdateCubemapSettings(*this);
}

void SRenderSystemDX11::SetCubemapAmount(float amount, ECubemapType type)
{
	switch (type)
	{
	case ECubemapType::Diffuse:
		diffuseCubemapAmount = std::clamp(amount, 0.0f, 1.0f);
		break;
	case ECubemapType::Specular:
		specularCubemapAmount = std::clamp(amount, 0.0f, 1.0f);
		break;
	}

	constantBuffers.UpdateCubemapSettings(*this);
}

float SRenderSystemDX11::GetCubemapAmount(ECubemapType type) const noexcept
{
	return (type == ECubemapType::Diffuse) ? diffuseCubemapAmount : specularCubemapAmount;
}

void SRenderSystemDX11::RemoveCubemap(ECubemapType type)
{
	textureManager.RemoveCubemap(type);
	constantBuffers.UpdateCubemapSettings(*this);
}

void SRenderSystemDX11::SetGlobalTint(const SColor3F& color)
{
	globalTint = SConvert::ToColor4(color);
	constantBuffers.UpdateSettingsBuffer(*this, world->GetCamera(),
		globalTint, backLight, pbrGammaCorrection);
}

void SRenderSystemDX11::SetBackLight(const SColor3F& color)
{
	backLight = SConvert::ToColor4(color);
	constantBuffers.UpdateSettingsBuffer(*this, world->GetCamera(),
		globalTint, backLight, pbrGammaCorrection);
}

void SRenderSystemDX11::SetGammaCorrection(const SColor3F& color)
{
	pbrGammaCorrection = SConvert::ToColor4(color);
	constantBuffers.UpdateSettingsBuffer(*this, world->GetCamera(),
		globalTint, backLight, pbrGammaCorrection);
}

void SRenderSystemDX11::LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate)
{
	meshManager.LoadStaticMeshInstances(path, groupId, delegate);
}

void SRenderSystemDX11::PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate)
{
	meshManager.PreloadStaticMeshes(path, delegate);
}

std::pair<std::vector<SMeshMaterial>, bool> SRenderSystemDX11::FindMeshMaterials(entt::entity entity) const
{
	std::vector<SMeshMaterial> materials;

	if (world)
	{
		const auto& registry = world->GetEntities();
		auto mesh = registry.try_get<SStaticMeshComponent>(entity);
		if (mesh)
		{
			if (FindMesh(mesh->id, nullptr, &materials, nullptr, nullptr))
			{
				return { materials, true };
			}
		}
	}

	return { materials, false };
}

const SShaderDataDX11* SRenderSystemDX11::FindShader(const std::string& name) const
{
	auto shaderIt = shaders.find(name);
	return (shaderIt == shaders.end()) ? nullptr : &shaderIt->second;
}

std::pair<ID3D11ShaderResourceView*, SSize2> SRenderSystemDX11::FindTexture(STexID id) const
{
	auto texture = static_cast<STextureDataDX11*>(textureManager.FindTexture(id));
	if (texture)
	{
		return { texture->view.Get(), texture->texSize };
	}

	return { nullptr, SConst::ZeroSSize2 };
}

ID3D11ShaderResourceView* SRenderSystemDX11::FindCubemap(ECubemapType type) const
{
	auto cubemap = static_cast<SCubemapDataDX11*>(textureManager.FindCubemap(type));
	if (cubemap)
	{
		return cubemap->view.Get();
	}

	return nullptr;
}

std::uint32_t SRenderSystemDX11::GetCubemapMaxMipLevel(ECubemapType type) const noexcept
{
	auto it = cubemapMIPLevels.find(type);
	if (it != cubemapMIPLevels.end())
	{
		return it->second;
	}

	return 1u;
}

bool SRenderSystemDX11::FindMesh(SMeshID id, DXGI_FORMAT* outFormat, std::vector<SMeshMaterial>* outMaterials, ID3D11Buffer** outVB, ID3D11Buffer** outIB) const
{
	auto mesh = static_cast<SMeshDataDX11*>(meshManager.FindMesh(id));
	if (mesh)
	{
		if (outMaterials) *outMaterials = mesh->materials;
		if (outFormat) *outFormat = mesh->ibFormat;
		if (outVB) *outVB = mesh->vb.Get();
		if (outIB) *outIB = mesh->ib.Get();
		return true;
	}

	return false;
}

void SRenderSystemDX11::Update(float deltaSeconds, const SAppContext& context)
{
	S_TRY

	shaderManager.Update();
	textureManager.Update();
	meshManager.Update();

	S_CATCH{ S_THROW("SRenderSystemDX11::Update()") }
}

void SRenderSystemDX11::Render(const SAppContext& context)
{
	S_TRY

	if (!deviceContext || !swapChain || !world)
	{
		throw std::exception("Wrong context");
	}

	auto& features = context.app->GetFeatures();
	auto [clearColor, bHasClearColor] = GetFeatureColor(features, SAppFeature::ClearScreenColor);

	// setup device context
	auto customRenderTarget = renderTargetView.Get();
	if (bAllowFXAA) customRenderTarget = sdrScene.GetRenderTargetView();

	deviceContext->OMSetRenderTargets(1, &customRenderTarget, depthStencilView.Get());
	deviceContext->ClearRenderTargetView(customRenderTarget, bHasClearColor ? SConvert::FromSColor3(clearColor) : SConst::White4F);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	drawCalls = 0;

	// render 3d frame
	constantBuffers.ApplyTransform3D(deviceContext.Get(), world->GetCamera(),
		cachedRenderSystemSize.width, cachedRenderSystemSize.height, context.gameTime);
	constantBuffers.UpdateSettingsBuffer(*this, world->GetCamera(),
		globalTint, backLight, pbrGammaCorrection);

	auto specularMap = static_cast<SCubemapDataDX11*>(textureManager.FindCubemap(ECubemapType::Specular));
	if (specularMap) deviceContext->PSSetShaderResources(4, 1, specularMap->view.GetAddressOf());
	auto diffuseMap = static_cast<SCubemapDataDX11*>(textureManager.FindCubemap(ECubemapType::Diffuse));
	if (diffuseMap) deviceContext->PSSetShaderResources(5, 1, diffuseMap->view.GetAddressOf());

	meshRender.Render(context.deltaSeconds);

	if (bAllowFXAA)
	{
		// render 3d scene to back buffer
		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
		fxaaRender.Render(sdrScene.GetShaderResourceView(), cachedRenderSystemSize);
	}

	// render 2d frame
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	constantBuffers.ApplyTransform2D(deviceContext.Get(), world->GetCamera(),
		cachedWorld2dScale, cachedRenderSystemSize.width, cachedRenderSystemSize.height);

	coloredSpriteRender.Render(context.deltaSeconds, context.gameTime);
	frameAnimSpriteRender.Render(context.deltaSeconds, context.gameTime);
	texturedSpriteRender.Render(context.deltaSeconds, context.gameTime);
	textRender.Render(context.deltaSeconds, context.gameTime);

	// present on screen
	HRESULT hRenderResult = S_FALSE;
	if (bAllowTearing && (appMode == SAppMode::Windowed))
	{
		hRenderResult = swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}
	else
	{
		const bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
		hRenderResult = swapChain->Present(bVSync ? 1 : 0, 0);
	}

	if (FAILED(hRenderResult))
	{
		DebugMsg("[%s] SRenderSystemDX11::Render(): render failed result: ",
			GetTimeStamp(std::chrono::system_clock::now()).c_str());

		switch (hRenderResult)
		{
		case DXGI_STATUS_OCCLUDED:
		case DXGI_ERROR_INVALID_CALL:
			DebugMsg("DXGI_STATUS_OCCLUDED or DXGI_ERROR_INVALID_CALL\n");
			if (context.app->IsWindowHasFocus())
			{
				Resize(cachedRenderSystemSize, context, true);
				DebugMsg("[%s] SRenderSystemDX11::Render(): Device reset\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str());
			}
			return;
		case DXGI_ERROR_DEVICE_REMOVED:
			DebugMsg("DXGI_ERROR_DEVICE_REMOVED\n");
			break;
		case DXGI_ERROR_DEVICE_RESET:
			DebugMsg("DXGI_ERROR_DEVICE_RESET\n");
			break;
		default:
			DebugMsg("0x%08x\n", static_cast<std::int32_t>(hRenderResult));
			break;
		}

		throw std::exception("Render failed");
	}
	else
	{
		if (bAllowHDR && (!dxGIFactory || !dxGIFactory->IsCurrent()))
		{
			UpdateColorSpace();
		}
	}

	if (bCachedNeedDebugTrace)
	{
		const auto worldScale = world->GetScale().GetScale();
		DebugMsg("[%s] SRenderSystemDX11::Render(): WorldScale=%.2fx%.2f Frame=%d, Time=%.1fs DrawCalls=%d FPS=[%d]\n\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), worldScale.x, worldScale.y,
			context.gameFrame, context.gameTime, drawCalls, context.fps);
	}

	cachedStats.numTextures = textureManager.GetNumTextures();

	S_CATCH{ S_THROW("SRenderSystemDX11::Render()") }
}

void SRenderSystemDX11::Subscribe(const SAppContext& inContext)
{
	SAppContext context = inContext;

	context.world->onLightsChanged.connect<&SRenderSystemDX11::OnLightsChanged>(this);
	context.world->GetCamera().onViewChanged.connect<&SRenderSystemDX11::OnCameraViewChanged>(this);
	context.world->GetScale().onScaleChanged.connect<&SRenderSystemDX11::OnWorldScaleChanged>(this);
}

void SRenderSystemDX11::OnWorldScaleChanged(SVector2 worldScale)
{
	cachedWorld2dScale = worldScale;
}

void SRenderSystemDX11::OnLightsChanged(const IWorld& world)
{
	if (deviceContext)
	{
		SLightsBuffer lightsData = world.GetLightsData();
		constantBuffers.UpdateLightSettings(*this, lightsData);
	}
}

void SRenderSystemDX11::OnCameraViewChanged(const SCamera& camera)
{
	UpdateCamera(camera);
}

void SRenderSystemDX11::UpdateCamera(const SCamera& camera)
{
	auto newPos2d = camera.GetPosition(SCameraSpace::Camera2D);
	auto newTarget2d = camera.GetTarget(SCameraSpace::Camera2D);
	auto newPos3d = camera.GetPosition(SCameraSpace::Camera3D);
	auto newTarget3d = camera.GetTarget(SCameraSpace::Camera3D);

	if (cachedCameraPos2d != newPos2d || cachedCameraTarget2d != newTarget2d)
	{
		cachedCameraTarget2d = newTarget2d;
		cachedCameraPos2d = newPos2d;
	}

	if (cachedCameraPos3d != newPos3d || cachedCameraTarget3d != newTarget3d)
	{
		cachedCameraTarget3d = newTarget3d;
		cachedCameraPos3d = newPos3d;
	}
}

bool SRenderSystemDX11::CanRender() const noexcept
{
	const bool bRenderSystemReady = (deviceContext && swapChain && renderTargetView);
	return bRenderSystemReady && !shaders.empty();
}

void SRenderSystemDX11::Clear(IWorld* world, bool removeRooted)
{
}

void SRenderSystemDX11::RequestResize(std::uint32_t width, std::uint32_t height)
{
	S_TRY

	DXGI_MODE_DESC displayModeDesc{};
	if (!FindDisplayMode(dxGIFactory, width, height, cachedMaxRefreshRate, &displayModeDesc))
	{
		throw std::exception("Cannot find display mode");
	}

	displayModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	displayModeDesc.Format = backBufferFormat;
	swapChain->ResizeTarget(&displayModeDesc);

	S_CATCH{ S_THROW("SRenderSystemDX11::RequestResize()") }
}

void SRenderSystemDX11::Resize(const SSize2& newSize, const SAppContext& context, bool bForceResize)
{
	S_TRY

	const bool bNeedResize = (cachedRenderSystemSize != newSize) || bForceResize;
	if (swapChain && bNeedResize)
	{
		// reset render system
		deviceContext->OMSetRenderTargets(0, NULL, NULL);
		if (bAllowHDR) hdrScene.Shutdown();
		else if (bAllowFXAA) sdrScene.Shutdown();
		renderTargetView.Reset();
		depthStencilView.Reset();
		renderTarget.Reset();
		depthStencil.Reset();
		deviceContext->Flush();

		// resize swap chain
		if (FAILED(swapChain->ResizeBuffers(
			SConst::DefaultBackBufferCount, newSize.width, newSize.height,
			SConst::DefaultBackBufferFormat, swapChainFlags)))
		{
			throw std::exception("Cannot resize swap chain");
		}

		CreateRenderTargetAndDepthStencil(newSize.width, newSize.height);

		if (bAllowHDR) hdrScene.Create(newSize);
		else if (bAllowFXAA) sdrScene.Create(newSize);

		SetMode(appMode);

		// update world settings
		auto newCameraPos = SVector3{ 0.5f * newSize.width, 0.5f * newSize.height, 1.0f };
		auto newCameraTarget = SVector3{ newCameraPos.x, newCameraPos.y, 0.0f };
		context.world->GetCamera().Set(SCameraSpace::Camera2D, newCameraPos, newCameraTarget);
		context.world->UpdateWorldScale(newSize);
		cachedRenderSystemSize = newSize;

		DebugMsg("SRenderSystemDX11::Resize(): resized to %dx%d\n", newSize.width, newSize.height);
	}

	S_CATCH{ S_THROW("SRenderSystemDX11::Resize()") }
}

void SRenderSystemDX11::SetMode(SAppMode mode)
{
	if (swapChain) swapChain->SetFullscreenState((mode == SAppMode::Fullscreen), nullptr);
	appMode = mode;
}

std::shared_ptr<STextureBase> SRenderSystemDX11::CreateTexture(const STextureData& textureData)
{
	D3D11_SUBRESOURCE_DATA initData = {
		textureData.data.data(),
		static_cast<UINT>(textureData.texSize.width * 4),
		static_cast<UINT>(textureData.data.size())
	};

	// create texture
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = textureData.texSize.width;
	desc.Height = textureData.texSize.height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> newTexture;
	if (SUCCEEDED(d3dDevice->CreateTexture2D(&desc, &initData, newTexture.GetAddressOf())))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		ComPtr<ID3D11ShaderResourceView> newView;
		if (SUCCEEDED(d3dDevice->CreateShaderResourceView(newTexture.Get(), &SRVDesc, newView.GetAddressOf())))
		{
			auto outTexture = std::make_shared<STextureDataDX11>();
			outTexture->texture = std::move(newTexture);
			outTexture->view = std::move(newView);
			outTexture->texSize = textureData.texSize;
			return outTexture;
		}
	}

	return nullptr;
}

std::shared_ptr<STextureBase> SRenderSystemDX11::CreateCubemap(const SCubemapData& cubemapData)
{
	auto outCubemap = std::make_shared<SCubemapDataDX11>();

	if (FAILED(DirectX::CreateDDSTextureFromMemory(
		d3dDevice.Get(), cubemapData.data.data(), cubemapData.data.size(),
		outCubemap->texture.GetAddressOf(), outCubemap->view.GetAddressOf())))
	{
		return nullptr;
	}

	if (outCubemap->view)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
		outCubemap->view->GetDesc(&desc);
		const std::uint32_t mipLevels = static_cast<std::uint32_t>(desc.TextureCube.MipLevels);

		bool bNeedUpdateConstants = false;
		auto it = cubemapMIPLevels.find(cubemapData.type);
		if (it == cubemapMIPLevels.end() || it->second != mipLevels)
		{
			bNeedUpdateConstants = true;
		}

		cubemapMIPLevels.insert_or_assign(cubemapData.type, mipLevels);

		if (bNeedUpdateConstants)
		{
			constantBuffers.UpdateCubemapSettings(*this);
		}
	}

	return outCubemap;
}

std::shared_ptr<SMeshBase> SRenderSystemDX11::CreateMesh(const SMesh& meshData)
{
	auto outMesh = std::make_shared<SMeshDataDX11>();

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(SVertex) * meshData.vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA bufferData{};
	bufferData.pSysMem = meshData.vertices.data();

	// create vertex buffer
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &bufferData, outMesh->vb.GetAddressOf())))
	{
		DebugMsg("[%s] SRenderSystemDX11::CreateMesh(): Cannot create vertex buffer\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str());
		return nullptr;
	}

	// create index buffer
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	if (meshData.indices16.empty())
	{
		bufferDesc.ByteWidth = sizeof(std::uint32_t) * meshData.indices32.size();
		bufferData.pSysMem = meshData.indices32.data();
		outMesh->ibFormat = DXGI_FORMAT_R32_UINT;
	}
	else
	{
		bufferDesc.ByteWidth = sizeof(std::uint16_t) * meshData.indices16.size();
		bufferData.pSysMem = meshData.indices16.data();
		outMesh->ibFormat = DXGI_FORMAT_R16_UINT;
	}


	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &bufferData, outMesh->ib.GetAddressOf())))
	{
		DebugMsg("[%s] SRenderSystemDX11::CreateMesh(): Cannot create index buffer\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str());
		return nullptr;
	}

	// load textures
	for (auto& material : meshData.materials)
	{
		STexID baseTexId = ResourceID<STexID>(material.baseTexture.string());
		STexID normTexId = ResourceID<STexID>(material.normTexture.string());
		STexID rmaTexId = ResourceID<STexID>(material.rmaTexture.string());
		STexID emiTexId = ResourceID<STexID>(material.emiTexture.string());
		SPathList paths;
		auto [baseView, baseSize] = FindTexture(baseTexId);
		if (!baseView && !material.baseTexture.empty()) paths.push_back(material.baseTexture);
		auto [normView, normSize] = FindTexture(normTexId);
		if (!normView && !material.normTexture.empty()) paths.push_back(material.normTexture);
		auto [ormView, ormSize] = FindTexture(rmaTexId);
		if (!ormView && !material.rmaTexture.empty()) paths.push_back(material.rmaTexture);
		auto [emiView, emiSize] = FindTexture(emiTexId);
		if (!ormView && !material.emiTexture.empty()) paths.push_back(material.emiTexture);

		if (!paths.empty())
		{
			outMesh->materials.emplace_back(baseTexId, normTexId, rmaTexId, emiTexId,
				material.firstIndex, material.numVertices, material.numIndices);

			static auto onLoaded = [](std::vector<STexID>&) {};
			textureManager.PreloadTextures(paths, onLoaded);
		}
	}

	DebugMsg("[%s] SRenderSystemDX11::CreateMesh(): mesh '%s' created and added to cache\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.name.c_str());

	return outMesh;
}

std::shared_ptr<SMeshBase> SRenderSystemDX11::CreateSkeletalMesh(const SMesh& data)
{
	return nullptr;
}

#endif // WIN32
