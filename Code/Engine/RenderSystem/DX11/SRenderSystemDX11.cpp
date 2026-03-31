/***************************************************************************
* SRenderSystemDX11.cpp
*/

#ifdef WIN32

#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/Windows/SWindowsUtils.h"
#include "Application/SApplicationInterface.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#pragma comment(lib, "d3d11.lib")


SRenderSystemDX11::SRenderSystemDX11()
	: coloredSpriteRender(*this)
	, texturedSpriteRender(*this)
	, frameAnimSpriteRender(*this)
	, textRenderSystem(*this)
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

	HWND hWnd = static_cast<HWND>(windowHandle);
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

	const bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	const bool bAllowFullscreen = GetFeatureFlag(features, SAppFeature::AllowFullscreen);

	// create device
	D3D_FEATURE_LEVEL createdLevel{};
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	ComPtr<ID3D11Device> newDevice;
	ComPtr<ID3D11DeviceContext> newDeviceContext;
	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, newDevice.GetAddressOf(), &createdLevel, newDeviceContext.GetAddressOf())))
	{
		throw std::exception("Cannot create device");
	}
	else
	{
		DebugMsg("[%s] SRenderSystemDX11::Create(): Created D3D_11.%d device\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(),
			(createdLevel == D3D_FEATURE_LEVEL_11_1) ? 1 : 0);
	}

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

	// get DXGI device
	ComPtr<IDXGIDevice> dxGIDevice;
	d3dDevice.As(&dxGIDevice);

	// get DXGI adapter
	ComPtr<IDXGIAdapter> dxGIAdapter;
	dxGIDevice->GetAdapter(&dxGIAdapter);

	// get DXGI factory (DXGI 1.4+)
	ComPtr<IDXGIFactory4> dxGIFactory;
	dxGIAdapter->GetParent(IID_PPV_ARGS(&dxGIFactory));

	// create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = SConst::DefaultBackBufferFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = bAllowFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0u;

	std::string swapEffectName;
	D3D11_FEATURE_DATA_D3D11_OPTIONS3 options3{};
	HRESULT hFeatureGetResult = d3dDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options3, sizeof(options3));
	if (SUCCEEDED(hFeatureGetResult) && options3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer)
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapEffectName = "EFFECT_FLIP_DISCARD";
	}
	else
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapEffectName = "EFFECT_FLIP_SEQUENTIAL";
	}

	ComPtr<IDXGISwapChain1> newSwapChain;
	if (FAILED(dxGIFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hWnd,
		&swapChainDesc, NULL, NULL, newSwapChain.GetAddressOf())))
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapEffectName = "EFFECT_DISCARD";

		if (FAILED(dxGIFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hWnd,
			&swapChainDesc, NULL, NULL, newSwapChain.GetAddressOf())))
		{
			throw std::exception("Cannot create swap chain");
		}
	}

	DebugMsg("[%s] SRenderSystemDX11::Create(): Created swapchain type: %s\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str(), swapEffectName.c_str());

	newSwapChain.As(&swapChain);

	CreateRenderTargetViewAndSwapChain(width, height);

	// set up rasterizer
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.AntialiasedLineEnable = true;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	if (FAILED(d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf())))
	{
		throw std::exception("LcRendCannot create rasterizer state");
	}

	deviceContext->RSSetState(rasterizerState.Get());

	// create blend state
	D3D11_BLEND_DESC blendStateDesc{};
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
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

	// init managers
	shaderManager.Init(context.pool);
	textureManager.Init(context.pool);

	cachedCameraPos = SVector3{ width / 2.0f, height / 2.0f, 1.0f };
	cachedCameraTarget = SVector3{ cachedCameraPos.x, cachedCameraPos.y, 0.0f };
	constantBuffers.Init(d3dDevice.Get(), deviceContext.Get(), cachedCameraPos, cachedCameraTarget, width, height);
	context.world->UpdateWorldScale(viewportSize);
	cachedRenderSystemSize = SSize2{ width, height };

	const bool bShouldGoFullscreen = (ScreenW == width && ScreenH == height);
	if (bShouldGoFullscreen || mode == SAppMode::Fullscreen)
	{
		SetMode(SAppMode::Fullscreen);
	}

	bCachedNeedDebugTrace = GetFeatureFlag(features, SAppFeature::RenderSystemDebugTrace);
	DebugMsg("[%s] SRenderSystemDX11::Create(): Render system created\n",
		GetTimeStamp(std::chrono::system_clock::now()).c_str());

	S_CATCH{ S_THROW("SRenderSystemDX11::Create()") }
}

void SRenderSystemDX11::CreateRenderTargetViewAndSwapChain(std::uint32_t width, std::uint32_t height)
{
	S_TRY

	if (!swapChain || !d3dDevice || !deviceContext)
	{
		throw std::exception("Invalid parameters");
	}

	// create back buffer and render target
	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
	{
		throw std::exception("Cannot get back buffer");
	}

	if (FAILED(d3dDevice->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf())))
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

	// set render targets
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

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
	textRenderSystem.Shutdown();
	frameAnimSpriteRender.Shutdown();
	texturedSpriteRender.Shutdown();
	coloredSpriteRender.Shutdown();
	constantBuffers.Shutdown();
	textureManager.Shutdown();
	shaderManager.Shutdown();
	rasterizerState.Reset();
	blendState.Reset();
	depthStencilView.Reset();
	depthStencilState.Reset();
	depthStencil.Reset();
	renderTargetView.Reset();
	deviceContext.Reset();
	d3dDevice.Reset();
	swapChain.Reset();
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
	shaderManager.LoadShaders(paths, [this](const SDXShaderManager::SCompiledShader& shaderData)
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
		else if (textRenderSystem.CheckShaderName(shaderData.name))
		{
			textRenderSystem.Setup(shader);
		}
		shader.vsCode = nullptr;

		shaders.emplace(shaderData.name, shader);
	});

	S_CATCH{ S_THROW("SRenderSystemDX11::LoadShaders()") }
}

STexID SRenderSystemDX11::LoadTexture(const std::filesystem::path& texturePath)
{
	return textureManager.LoadTexture(texturePath);
}

void SRenderSystemDX11::PreLoadTextures(const SPathList& paths, OnPreLoadTexturesDelegate delegate)
{
	textureManager.PreLoadTextures(paths, delegate);
}

const SShaderDataDX11* SRenderSystemDX11::FindShader(const std::string& name) const
{
	auto shaderIt = shaders.find(name);
	return (shaderIt == shaders.end()) ? nullptr : &shaderIt->second;
}


std::pair<ID3D11ShaderResourceView*, SSize2> SRenderSystemDX11::FindTexture(STexID id) const
{
	SSize2 size{};
	ID3D11Texture2D* texture = nullptr;
	ID3D11ShaderResourceView* view = nullptr;
	if (textureManager.FindTexture(id, &texture, &view, &size))
	{
		return { view, size };
	}

	return { view, size };
}

void SRenderSystemDX11::Update(float deltaSeconds, const SAppContext& context)
{
	S_TRY

	shaderManager.Update();
	textureManager.Update(d3dDevice.Get());

	S_CATCH{ S_THROW("SRenderSystemDX11::Update()") }
}

void SRenderSystemDX11::Render(const SAppContext& context)
{
	S_TRY

	if (!deviceContext || !swapChain || !world)
	{
		throw std::exception("Invalid render device");
	}

	// setup device
	auto& features = context.app->GetFeatures();
	auto [clearColor, bHasClearColor] = GetFeatureColor(features, SAppFeature::ClearScreenColor);
	if (bHasClearColor)
	{
		deviceContext->ClearRenderTargetView(renderTargetView.Get(), SConvert::FromSColor3(clearColor));
	}

	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	// render frame
	drawCalls = 0;
	coloredSpriteRender.Render(context.deltaSeconds, context.gameTime);
	texturedSpriteRender.Render(context.deltaSeconds, context.gameTime);
	frameAnimSpriteRender.Render(context.deltaSeconds, context.gameTime);
	textRenderSystem.Render(context.deltaSeconds, context.gameTime, context.text);

	const bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	HRESULT hRenderResult = swapChain->Present(bVSync ? 1 : 0, 0);
	if (FAILED(hRenderResult))
	{
		DebugMsg("[%s] SRenderSystemDX11::Render(): render failed result: %d\n\n", static_cast<std::int32_t>(hRenderResult));

		throw std::exception("Render failed");
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

	context.world->onGlobalTintChanged.connect<&SRenderSystemDX11::OnGlobalTintChanged>(this);
	context.world->GetCamera().onViewChanged.connect<&SRenderSystemDX11::OnCameraViewChanged>(this);
	context.world->GetScale().onScaleChanged.connect<&SRenderSystemDX11::OnWorldScaleChanged>(this);
}

void SRenderSystemDX11::OnGlobalTintChanged(SColor3 globalTint)
{
	if (deviceContext && constantBuffers.settingsBuffer)
	{
		SSettingsBuffer settings{};
		settings.worldTint = SConvert::ToVector4(globalTint);
		deviceContext->UpdateSubresource(constantBuffers.settingsBuffer.Get(), 0, NULL, &settings, 0, 0);
	}
}

void SRenderSystemDX11::OnWorldScaleChanged(SVector2 worldScale)
{
	if (deviceContext && constantBuffers.transMatrixBuffer)
	{
		SSingleMatrixBuffer worldTrans{};
		worldTrans.mat = SMath::ScaleMatrix2(worldScale);
		deviceContext->UpdateSubresource(constantBuffers.transMatrixBuffer.Get(), 0, NULL, &worldTrans, 0, 0);
	}
}

void SRenderSystemDX11::OnCameraViewChanged(const SCamera& camera)
{
	UpdateCamera(camera.GetPosition(), camera.GetTarget());
}

void SRenderSystemDX11::UpdateCamera(SVector3 newPos, SVector3 newTarget)
{
	if (deviceContext &&
		constantBuffers.viewMatrixBuffer && (
		cachedCameraPos != newPos || cachedCameraTarget != newTarget))
	{
		SMatrix4 view = SMath::LookAtMatrix(newPos, newTarget);
		deviceContext->UpdateSubresource(constantBuffers.viewMatrixBuffer.Get(), 0, NULL, view.m, 0, 0);
		cachedCameraPos = newPos;
		cachedCameraTarget = newTarget;
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
	if (!SFindDisplayMode(width, height, cachedMaxRefreshRate, &displayModeDesc))
	{
		throw std::exception("Cannot find display mode");
	}

	swapChain->ResizeTarget(&displayModeDesc);

	S_CATCH{ S_THROW("SRenderSystemDX11::RequestResize()") }
}

void SRenderSystemDX11::Resize(std::uint32_t width, std::uint32_t height, const SAppContext& context)
{
	S_TRY

	SSize2 newViewportSize{ width, height };
	bool needResize = (cachedRenderSystemSize != newViewportSize);

	if (swapChain && needResize)
	{
		// reset render system
		deviceContext->OMSetRenderTargets(0, NULL, NULL);
		depthStencilView.Reset();
		depthStencil.Reset();
		renderTargetView.Reset();

		// resize swap chain
		auto& features = context.app->GetFeatures();
		const bool bAllowFullscreen = GetFeatureFlag(features, SAppFeature::AllowFullscreen);
		const UINT flags = bAllowFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0u;
		if (FAILED(swapChain->ResizeBuffers(2, width, height, SConst::DefaultBackBufferFormat, flags)))
		{
			throw std::exception("Cannot resize swap chain");
		}

		CreateRenderTargetViewAndSwapChain(width, height);

		// update world settings
		auto newCameraPos = SVector3{ width / 2.0f, height / 2.0f, 1.0f };
		auto newCameraTarget = SVector3{ newCameraPos.x, newCameraPos.y, 0.0f };
		context.world->GetCamera().Set(newCameraPos, newCameraTarget);
		context.world->UpdateWorldScale(newViewportSize);
		cachedRenderSystemSize = newViewportSize;

		// update projection matrix
		SMatrix4 proj = SMath::OrthoMatrix(newViewportSize, 1.0f, 0.0f);
		deviceContext->UpdateSubresource(constantBuffers.projMatrixBuffer.Get(), 0, NULL, proj.m, 0, 0);

		DebugMsg("SRenderSystemDX11::Resize(): resized to %dx%d\n", newViewportSize.width, newViewportSize.height);
	}

	S_CATCH{ S_THROW("SRenderSystemDX11::Resize()") }
}

void SRenderSystemDX11::SetMode(SAppMode mode)
{
	if (swapChain) swapChain->SetFullscreenState((mode == SAppMode::Fullscreen), nullptr);
}

#endif // WIN32
