/***************************************************************************
* SRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"
#include "RenderSystem/Windows/SDXShaderManager.h"
#include "RenderSystem/Windows/SWindowsUtils.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/DX11/SColoredSpriteRendererDX11.h"
#include "RenderSystem/DX11/STexturedSpriteRendererDX11.h"

#include <d3d11_4.h>
#include <dxgi1_4.h>
#include <directxmath.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

#pragma warning(disable : 4251)


/***************************************************************************
* DirectX 11 render system
*/
class SRenderSystemDX11 : public IRenderSystem
{
public:
	//
	SRenderSystemDX11();
	//
	SShaderDataDX11* FindShader(const std::string& name);
	//
	SConstantBuffersDX11& GetConstantBuffers() noexcept { return constantBuffers; }
	//
	ID3D11DeviceContext* GetD3D11DeviceContext() const noexcept { return deviceContext.Get(); }
	//
	ID3D11Device* GetD3D11Device() const noexcept { return d3dDevice.Get(); }
	//
	IWorld* GetWorld() const noexcept { return world; }
	//
	bool IsNeedDebugTrace() const noexcept { return bCachedNeedDebugTrace; }


public:// IRenderSystem interface implementation
	//
	virtual ~SRenderSystemDX11() override;
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
	virtual void Clear(IWorld* world, bool removeRooted = false) override;
	//
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) override;
	//
	virtual void Resize(std::uint32_t width, std::uint32_t height, const SAppContext& context) override;
	//
	virtual void SetMode(SAppMode mode) override;
	//
	virtual void UpdateCamera(SVector3 newPos, SVector3 newTarget) override;
	//
	virtual void AddDrawCalls(std::uint32_t inDrawCalls) noexcept { drawCalls += inDrawCalls; }
	//
	virtual SSize2 GetRenderSize() const noexcept override { return cachedRenderSystemSize; }
	//
	virtual SRSStats GetStats() const noexcept override { return SRSStats{}; }
	//
	virtual SRSType GetType() const noexcept override { return SRSType::DX11; }


protected:
	//
	void CreateRenderTargetViewAndSwapChain(std::uint32_t width, std::uint32_t height);
	//
	void OnGlobalTintChanged(SColor3 globalTint);
	//
	void OnWorldScaleChanged(SVector2 worldScale);
	//
	void OnCameraViewChanged(const SCamera& camera);


protected:
	//
	SColoredSpriteRendererDX11 coloredSpriteRendererDX11;
	//
	STexturedSpriteRendererDX11 texturedSpriteRendererDX11;
	//
	SConstantBuffersDX11 constantBuffers;
	//
	SDXShaderManager shaderManager;
	//
	STextureManagerDX11 textureManager;
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


protected:
	//
	std::unordered_map<std::string, SShaderDataDX11> shaders;
	//
	IWorld* world{};
	//
	std::uint32_t drawCalls = 0;


protected:
	//
	SSize2 cachedRenderSystemSize{};
	//
	SVector3 cachedCameraPos{};
	//
	SVector3 cachedCameraTarget{};
	//
	std::uint32_t cachedMaxRefreshRate = 0;
	//
	bool bCachedNeedDebugTrace = false;

};
