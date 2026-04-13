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

#pragma comment(lib, "d3d11.lib")


SRenderSystemDX11::SRenderSystemDX11()
	: coloredSpriteRender(*this)
	, texturedSpriteRender(*this)
	, frameAnimSpriteRender(*this)
	, textRender(*this)
	, meshRender(*this)
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
		throw std::exception("Cannot create rasterizer state");
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

	if (FAILED(d3dDevice->CreateSamplerState(&desc, IBLSampler.GetAddressOf())))
	{
		throw std::exception("Cannot create IBL sampler");
	}

	static ID3D11SamplerState* samplers[] = { surfaceSampler.Get(), IBLSampler.Get() };
	deviceContext->PSSetSamplers(0, 2, samplers);

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
	meshRender.Shutdown();
	textRender.Shutdown();
	frameAnimSpriteRender.Shutdown();
	texturedSpriteRender.Shutdown();
	coloredSpriteRender.Shutdown();
	constantBuffers.Shutdown();
	meshManager.Shutdown();
	textureManager.Shutdown();
	shaderManager.Shutdown();
	rasterizerState.Reset();
	surfaceSampler.Reset();
	IBLSampler.Reset();
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

ID3D11ShaderResourceView* SRenderSystemDX11::FindCubemap(ECubemapType type) const
{
	auto cubemap = static_cast<SCubemapDataDX11*>(textureManager.FindCubemap(type));
	if (cubemap)
	{
		return cubemap->view.Get();
	}

	return nullptr;
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

	// setup device
	auto& features = context.app->GetFeatures();
	auto [clearColor, bHasClearColor] = GetFeatureColor(features, SAppFeature::ClearScreenColor);
	if (bHasClearColor)
	{
		deviceContext->ClearRenderTargetView(renderTargetView.Get(), SConvert::FromSColor3(clearColor));
	}

	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
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

	// render 2d frame
	constantBuffers.ApplyTransform2D(deviceContext.Get(), world->GetCamera(),
		cachedWorld2dScale, cachedRenderSystemSize.width, cachedRenderSystemSize.height);

	coloredSpriteRender.Render(context.deltaSeconds, context.gameTime);
	texturedSpriteRender.Render(context.deltaSeconds, context.gameTime);
	frameAnimSpriteRender.Render(context.deltaSeconds, context.gameTime);
	textRender.Render(context.deltaSeconds, context.gameTime);

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
		context.world->GetCamera().Set(SCameraSpace::Camera2D, newCameraPos, newCameraTarget);
		context.world->UpdateWorldScale(newViewportSize);
		cachedRenderSystemSize = newViewportSize;

		DebugMsg("SRenderSystemDX11::Resize(): resized to %dx%d\n", newViewportSize.width, newViewportSize.height);
	}

	S_CATCH{ S_THROW("SRenderSystemDX11::Resize()") }
}

void SRenderSystemDX11::SetMode(SAppMode mode)
{
	if (swapChain) swapChain->SetFullscreenState((mode == SAppMode::Fullscreen), nullptr);
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

			static auto onLoaded = [](std::vector<std::filesystem::path>& textures)
			{
				for (auto& texture : textures)
				{
					DebugMsg("[%s] SRenderSystemDX11::CreateMesh(): material texture loaded '%s'\n",
						GetTimeStamp(std::chrono::system_clock::now()).c_str(), texture.string().c_str());
				}
			};
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
