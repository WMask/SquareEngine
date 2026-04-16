/***************************************************************************
* SRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/DX11/SColoredSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/STexturedSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/STextRenderSystemDX11.h"
#include "RenderSystem/DX11/SMeshRenderSystemDX11.h"
#include "RenderSystem/DX11/SSkeletalMeshRenderSystemDX11.h"
#include "RenderSystem/DX11/SFXAARenderSystemDX11.h"
#include "RenderSystem/Windows/SMeshManagerWindows.h"
#include "RenderSystem/Windows/STextureManagerWindows.h"
#include "RenderSystem/Windows/SShaderManagerWindows.h"
#include "RenderSystem/Windows/SUtilsWindows.h"

#include <d3d11_4.h>
#include <directxmath.h>

#pragma warning(disable : 4251)


/***************************************************************************
* DirectX 11 render system
*/
class SRenderSystemDX11
	: public IRenderSystemDX11
	, public ITextureLifetime
	, public IMeshLifetime
{
public:
	//
	SRenderSystemDX11();


public:// IRenderSystemDX11 interface implementation
	//
	virtual const SShaderDataDX11* FindShader(const std::string& name) const override;
	//
	virtual std::pair<ID3D11ShaderResourceView*, SSize2> FindTexture(STexID id) const override;
	//
	virtual ID3D11ShaderResourceView* FindCubemap(ECubemapType type) const override;
	//
	virtual std::uint32_t GetCubemapMaxMipLevel(ECubemapType type) const noexcept override;
	//
	virtual bool FindMesh(SMeshID id, DXGI_FORMAT* outFormat, std::vector<SMeshMaterial>* outMaterials,
		ID3D11Buffer** outVB, ID3D11Buffer** outIB) const override;
	//
	virtual SConstantBuffersDX11& GetConstantBuffers() noexcept override { return constantBuffers; }
	//
	virtual STextureManagerWindows& GetTextureManager() noexcept override { return textureManager; }
	//
	virtual ID3D11DeviceContext* GetDeviceContext() const noexcept override { return deviceContext.Get(); }
	//
	virtual ID3D11Device* GetDevice() const noexcept override { return d3dDevice.Get(); }
	//
	virtual IWorld* GetWorld() const noexcept override { return world; }
	//
	virtual bool IsNeedDebugTrace() const noexcept override { return bCachedNeedDebugTrace; }


public:// IRenderSystem interface implementation
	//
	virtual ~SRenderSystemDX11() override;
	//
	virtual STexID LoadTexture(const std::filesystem::path& texturePath) override;
	//
	virtual void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate) override;
	//
	virtual std::pair<SSize2, bool> GetTextureSize(STexID id) const override;
	//
	virtual void LoadCubemap(const std::filesystem::path& path, ECubemapType type) override;
	//
	virtual void RemoveCubemap(ECubemapType type) override;
	//
	virtual void SetCubemapAmount(float amount, ECubemapType type) override;
	//
	virtual float GetCubemapAmount(ECubemapType type) const noexcept override;
	//
	virtual void SetGlobalTint(const SColor3F& color) override;
	//
	virtual SColor3F GetGlobalTint() const override { return globalTint; }
	//
	virtual void SetBackLight(const SColor3F& color) override;
	//
	virtual SColor3F GetBackLight() const noexcept { return backLight; }
	//
	virtual void SetGammaCorrection(const SColor3F& color) override;
	//
	virtual SColor3F GetGammaCorrection() const noexcept { return pbrGammaCorrection; }
	//
	virtual void LoadStaticMeshInstances(const std::filesystem::path & path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate) override;
	//
	virtual void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate) override;
	//
	virtual void LoadSkeletalMesh(const std::filesystem::path& path, OnSkeletalMeshLoadedDelegate delegate) override;
	//
	virtual std::pair<std::vector<SMeshMaterial>, bool> FindMeshMaterials(entt::entity entity) const override;
	//
	virtual void Clear(IWorld* world, bool removeRooted = false) override;
	//
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) override;
	//
	virtual void SetMode(SAppMode mode) override;
	//
	virtual void UpdateCamera(const SCamera& camera) override;
	//
	virtual SSize2 GetScreenSize() const noexcept override { return cachedRenderSystemSize; }
	//
	virtual SRSStats GetStats() const noexcept override { return cachedStats; }
	//
	virtual SRSType GetType() const noexcept override { return SRSType::DX11; }


public:// IRenderSystemEx interface implementation
	//
	virtual void Create(HWND inHWnd, SAppMode mode, const SAppContext& context) override;
	//
	virtual void Shutdown() override;
	//
	virtual void Subscribe(const SAppContext& context) override;
	//
	virtual void LoadShaders(const std::filesystem::path& folderPath) override;
	//
	virtual void Update(float deltaSeconds, const SAppContext& context) override;
	//
	virtual void Render(const SAppContext& context) override;
	//
	virtual bool CanRender() const noexcept override;
	//
	virtual void Resize(const SSize2& size, const SAppContext& context, bool bForceResize = false) override;
	//
	virtual void AddDrawCalls(std::uint32_t inDrawCalls) noexcept { drawCalls += inDrawCalls; }


protected:// ITextureLifetime interface implementation
	//
	virtual std::shared_ptr<STextureBase> CreateTexture(const STextureData& data) override;
	//
	virtual std::shared_ptr<STextureBase> CreateCubemap(const SCubemapData& data) override;


protected:// IMeshLifetime interface implementation
	//
	virtual std::shared_ptr<SMeshBase> CreateMesh(const SMesh& data) override;
	//
	virtual std::shared_ptr<SMeshBase> CreateSkeletalMesh(const SSkeletalMesh& data) override;


protected:
	//
	std::shared_ptr<SMeshBase> CreateAnyMesh(const SMesh& data, const SSkeletalMesh& skData);
	//
	void CreateRenderTargetAndDepthStencil(std::uint32_t width, std::uint32_t height);
	//
	bool IsDisplayHDR10(IDXGIFactory2* factory);
	//
	void OnWorldScaleChanged(SVector2 worldScale);
	//
	void OnCameraViewChanged(const SCamera& camera);
	//
	void OnLightsChanged(const IWorld& world);
	//
	void UpdateColorSpace();


protected:
	//
	ComPtr<IDXGIFactory2> dxGIFactory;
	//
	ComPtr<ID3D11Device> d3dDevice;
	//
	ComPtr<IDXGISwapChain4> swapChain;
	//
	ComPtr<ID3D11DeviceContext> deviceContext;
	//
	ComPtr<ID3D11Texture2D> renderTarget;
	//
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	//
	ComPtr<ID3D11Texture2D> depthStencil;
	//
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	//
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	//
	ComPtr<ID3D11BlendState> blendState;
	//
	ComPtr<ID3D11RasterizerState> rasterizerState;
	//
	ComPtr<ID3D11SamplerState> surfaceSampler;
	//
	ComPtr<ID3D11SamplerState> cubemapSampler;
	//
	ComPtr<ID3D11SamplerState> pointSampler;


protected:
	//
	using TCubemapMIPLevels = std::unordered_map<ECubemapType, std::uint32_t>;
	//
	SColoredSpriteRenderSystemDX11 coloredSpriteRender;
	//
	STexturedSpriteRenderSystemDX11 texturedSpriteRender;
	//
	SFrameAnimSpriteRenderSystemDX11 frameAnimSpriteRender;
	//
	STextRenderSystemDX11 textRender;
	//
	SMeshRenderSystemDX11 meshRender;
	//
	SSkeletalMeshRenderSystemDX11 skMeshRender;
	//
	SFXAARenderSystemDX11 fxaaRender;
	//
	SConstantBuffersDX11 constantBuffers;
	//
	STextureManagerWindows textureManager;
	//
	SShaderManagerWindows shaderManager;
	//
	SMeshManagerWindows meshManager;
	//
	SRenderTarget sdrScene;
	//
	SRenderTarget hdrScene;
	//
	TCubemapMIPLevels cubemapMIPLevels;


protected:
	//
	std::unordered_map<std::string, SShaderDataDX11> shaders;
	//
	DXGI_FORMAT backBufferFormat = SConst::DefaultBackBufferFormat;
	//
	DXGI_COLOR_SPACE_TYPE colorSpace = SConst::DefaultSDRColorSpace;
	//
	std::uint32_t drawCalls = 0;
	//
	UINT swapChainFlags = 0u;
	//
	BOOL bFlipPresent = FALSE;
	//
	BOOL bAllowTearing = FALSE;
	//
	BOOL bAllowFXAA = FALSE;
	//
	BOOL bAllowHDR = FALSE;
	//
	SAppMode appMode;
	//
	IWorld* world{};
	//
	HWND hWnd{};


protected:
	//
	float diffuseCubemapAmount = 1.0f;
	//
	float specularCubemapAmount = 1.0f;
	//
	SColor4F globalTint = SConst::DefaultRenderSettings.globalTint;
	//
	SColor4F backLight = SConst::DefaultRenderSettings.backLight;
	//
	SColor4F pbrGammaCorrection = SConst::DefaultRenderSettings.pbrGammaCorrection;


protected:
	//
	SRSStats cachedStats{};
	//
	SSize2 cachedRenderSystemSize{};
	//
	SVector2 cachedWorld2dScale{};
	//
	SVector3 cachedCameraPos2d{};
	//
	SVector3 cachedCameraTarget2d{};
	//
	SVector3 cachedCameraPos3d{};
	//
	SVector3 cachedCameraTarget3d{};
	//
	std::uint32_t cachedMaxRefreshRate = 0;
	//
	bool bCachedNeedDebugTrace = false;

};
