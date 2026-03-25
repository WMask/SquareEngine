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
	maxRefreshRate = GetDeviceCaps(hDC, VREFRESH);
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

	// find display mode
	DXGI_MODE_DESC displayModeDesc{};
	if (!SFindDisplayMode(width, height, maxRefreshRate, &displayModeDesc))
	{
		throw std::exception("Cannot find display mode");
	}

	// create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc = displayModeDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = bVSync ? DXGI_SWAP_EFFECT_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = (mode == SAppMode::Windowed);
	swapChainDesc.Flags = bAllowFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), d3dDevice.GetAddressOf(),
		NULL, deviceContext.GetAddressOf())))
	{
		featureLevel = D3D_FEATURE_LEVEL_11_0;

		if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
			D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), d3dDevice.GetAddressOf(),
			NULL, deviceContext.GetAddressOf())))
		{
			throw std::exception("Cannot create device");
		}

		DebugMsg("SRenderSystemDX11::Create(): Created D3D_11.0 device\n");
	}
	else
	{
		DebugMsg("SRenderSystemDX11::Create(): Created D3D_11.1 device\n");
	}

	CreateRenderTargetViewAndSwapChain(width, height);

	// set up rasterizer
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.AntialiasedLineEnable = true;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
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

	cameraPos = SVector3{ width / 2.0f, height / 2.0f, 1.0f };
	cameraTarget = SVector3{ cameraPos.x, cameraPos.y, 0.0f };
	constantBuffers.Init(d3dDevice.Get(), deviceContext.Get(), cameraPos, cameraTarget, width, height);
	context.world->UpdateWorldScale(viewportSize);
	renderSystemSize = SSize2{ width, height };

	const bool bShouldGoFullscreen = (ScreenW == width && ScreenH == height);
	if (bShouldGoFullscreen || context.app->GetWindowMode() == SAppMode::Fullscreen)
	{
		SetMode(SAppMode::Fullscreen);
	}

	bNeedDebugTrace = GetFeatureFlag(features, SAppFeature::RenderSystemDebugTrace);
	DebugMsg("SRenderSystemDX11::Create(): Render system created\n");

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
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
	{
		throw std::exception("Cannot get back buffer");
	}

	if (FAILED(d3dDevice->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf())))
	{
		throw std::exception("Cannot create render target");
	}

	backBuffer.Reset();

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
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	if (FAILED(d3dDevice->CreateTexture2D(&depthBufferDesc, NULL, depthStencilBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create depth stencil buffer");
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
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
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if (FAILED(d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(),
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
	coloredSpriteRendererDX11.Shutdown();
	constantBuffers.Shutdown();
	shaderManager.Shutdown();
	rasterizerState.Reset();
	blendState.Reset();
	depthStencilView.Reset();
	depthStencilState.Reset();
	depthStencilBuffer.Reset();
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

		if (coloredSpriteRendererDX11.CheckShaderName(shaderData.name))
		{
			shader.vsCode = shaderData.vsCode.Get();
			coloredSpriteRendererDX11.Setup(*this, shader);
			shader.vsCode = nullptr;
		}

		shaders.emplace(shaderData.name, shader);
	});

	S_CATCH{ S_THROW("SRenderSystemDX11::LoadShaders()") }
}

SShaderDataDX11* SRenderSystemDX11::FindShader(const std::string& name)
{
	auto shaderIt = shaders.find(name);
	return (shaderIt == shaders.end()) ? nullptr : &shaderIt->second;
}

void SRenderSystemDX11::Update(float deltaSeconds, const SAppContext& context)
{
	S_TRY

	shaderManager.Update();

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
	auto [clearColor, bHasClearColor] = GetClearColor(features);
	if (bHasClearColor)
	{
		deviceContext->ClearRenderTargetView(renderTargetView.Get(), SConvert::FromSColor3(clearColor));
	}

	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// render frame
	coloredSpriteRendererDX11.Render(*this);

	const bool bVSync = GetFeatureFlag(features, SAppFeature::VSync);
	swapChain->Present(bVSync ? DXGI_SWAP_EFFECT_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD, 0);

	if (bNeedDebugTrace)
	{
		DebugMsg("SRenderSystemDX11::Render(): WorldScale=%.2fx%.2f Frame=%d, Time=%.1fs FPS=[%d]\n",
			world->GetWorldScale().GetScale().x, world->GetWorldScale().GetScale().y,
			context.gameFrame, context.gameTime, context.fps);
	}

	S_CATCH{ S_THROW("SRenderSystemDX11::Render()") }
}

void SRenderSystemDX11::Subscribe(const SAppContext& inContext)
{
	SAppContext context = inContext;

	context.world->onTintChanged.connect<&SRenderSystemDX11::OnTintChanged>(this);
	context.world->GetCamera().onViewChanged.connect<&SRenderSystemDX11::OnCameraViewChanged>(this);
	context.world->GetWorldScale().onScaleChanged.connect<&SRenderSystemDX11::OnWorldScaleChanged>(this);
}

void SRenderSystemDX11::OnTintChanged(SColor3 globalTint)
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
		cameraPos != newPos || cameraTarget != newTarget))
	{
		SMatrix4 view = SMath::LookAtMatrix(newPos, newTarget);
		deviceContext->UpdateSubresource(constantBuffers.viewMatrixBuffer.Get(), 0, NULL, view.m, 0, 0);
		cameraPos = newPos;
		cameraTarget = newTarget;
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
	if (!SFindDisplayMode(width, height, maxRefreshRate, &displayModeDesc))
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
	bool needResize = (renderSystemSize != newViewportSize);

	if (swapChain && needResize)
	{
		// reset render system
		deviceContext->OMSetRenderTargets(0, NULL, NULL);
		depthStencilView.Reset();
		depthStencilBuffer.Reset();
		renderTargetView.Reset();

		// resize swap chain
		auto& features = context.app->GetFeatures();
		const bool bAllowFullscreen = GetFeatureFlag(features, SAppFeature::AllowFullscreen);
		const UINT flags = bAllowFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0u;
		if (FAILED(swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, flags)))
		{
			throw std::exception("Cannot resize swap chain");
		}

		CreateRenderTargetViewAndSwapChain(width, height);

		// update world settings
		auto newCameraPos = SVector3{ width / 2.0f, height / 2.0f, 1.0f };
		auto newCameraTarget = SVector3{ newCameraPos.x, newCameraPos.y, 0.0f };
		context.world->GetCamera().Set(newCameraPos, newCameraTarget);
		context.world->UpdateWorldScale(newViewportSize);
		renderSystemSize = newViewportSize;

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

std::pair<SColor3, bool> SRenderSystemDX11::GetClearColor(const SAppFeaturesMap& features)
{
	auto colorIt = features.find(SAppFeature::ClearScreenColor);
	if (colorIt->second.has_value())
	{
		return { std::any_cast<SColor3>(colorIt->second), true };
	}

	return { SConst::OneSColor3, false };
}

#endif // WIN32
