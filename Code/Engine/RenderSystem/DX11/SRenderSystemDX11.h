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
#include "RenderSystem/Windows/SMeshManagerWindows.h"
#include "RenderSystem/Windows/STextureManagerWindows.h"
#include "RenderSystem/Windows/SShaderManagerWindows.h"
#include "RenderSystem/Windows/SUtilsWindows.h"

#include <d3d11_4.h>
#include <dxgi1_4.h>
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
	virtual ID3D11ShaderResourceView* FindCubemap(ECubemapType type) const override;
	//
	virtual std::pair<ID3D11ShaderResourceView*, SSize2> FindTexture(STexID id) const override;
	//
	virtual bool FindMesh(SMeshID id, std::vector<SMeshMaterial>* outMaterials,
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
	virtual void LoadCubemap(const std::filesystem::path& path, ECubemapType type) override;
	//
	virtual void RemoveCubemap(ECubemapType type) override;
	//
	virtual void SetCubemapAmount(float amount, ECubemapType type) override;
	//
	virtual float GetCubemapAmount(ECubemapType type) const noexcept override;
	//
	virtual void SetIBLAmount(float amount) override { IBLAmount = amount; }
	//
	virtual float GetIBLAmount() const noexcept { return IBLAmount; }
	//
	virtual void LoadStaticMeshInstances(const std::filesystem::path & path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate) override;
	//
	virtual void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate) override;
	//
	virtual void Clear(IWorld* world, bool removeRooted = false) override;
	//
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) override;
	//
	virtual void SetMode(SAppMode mode) override;
	//
	virtual void UpdateCamera(const SCamera& camera) override;
	//
	virtual SSize2 GetRenderSize() const noexcept override { return cachedRenderSystemSize; }
	//
	virtual SRSStats GetStats() const noexcept override { return cachedStats; }
	//
	virtual SRSType GetType() const noexcept override { return SRSType::DX11; }


public:// IRenderSystemEx interface implementation
	//
	virtual void Create(void* windowHandle, SAppMode mode, const SAppContext& context) override;
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
	virtual void Resize(std::uint32_t width, std::uint32_t height, const SAppContext& context) override;
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
	virtual std::shared_ptr<SMeshBase> CreateSkeletalMesh(const SMesh& data) override;


protected:
	//
	void CreateRenderTargetViewAndSwapChain(std::uint32_t width, std::uint32_t height);
	//
	void OnGlobalTintChanged(SColor3 globalTint);
	//
	void OnWorldScaleChanged(SVector2 worldScale);
	//
	void OnCameraViewChanged(const SCamera& camera);
	//
	void OnLightsChanged(const IWorld& world);


protected:
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
	SConstantBuffersDX11 constantBuffers;
	//
	STextureManagerWindows textureManager;
	//
	SShaderManagerWindows shaderManager;
	//
	SMeshManagerWindows meshManager;
	//
	ComPtr<IDXGISwapChain4> swapChain;
	//
	ComPtr<ID3D11Device> d3dDevice;
	//
	ComPtr<ID3D11DeviceContext> deviceContext;
	//
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	//
	ComPtr<ID3D11Texture2D> depthStencil;
	//
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	//
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	//
	ComPtr<ID3D11BlendState> blendState;
	//
	ComPtr<ID3D11RasterizerState> rasterizerState;
	//
	ComPtr<ID3D11SamplerState> surfaceSampler;
	//
	ComPtr<ID3D11SamplerState> IBLSampler;


protected:
	//
	std::unordered_map<std::string, SShaderDataDX11> shaders;
	//
	std::uint32_t drawCalls = 0;
	//
	float diffuseCubemapAmount = 1.0f;
	//
	float specularCubemapAmount = 1.0f;
	//
	float IBLAmount = 0.5f;
	//
	IWorld* world{};


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
