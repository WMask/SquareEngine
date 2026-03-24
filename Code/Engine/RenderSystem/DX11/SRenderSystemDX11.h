/***************************************************************************
* SRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"
#include "RenderSystem/Windows/SDXShaderManager.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/DX11/SColoredSpriteRendererDX11.h"

#include <d3d11.h>
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
	bool IsNeedDebugTrace() const noexcept { return bNeedDebugTrace; }


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
	virtual bool CanRender() const override;
	//
	virtual void Clear(IWorld* world, bool removeRooted = false) override;
	//
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) override;
	//
	virtual void Resize(std::uint32_t width, std::uint32_t height, const SAppContext& context) override;
	//
	virtual void SetMode(SAppMode mode) override;
	//
	virtual void UpdateCamera(float deltaSeconds, SVector3 newPos, SVector3 newTarget) override;
	//
	virtual SRSStats GetStats() const override { return SRSStats{}; }
	//
	virtual SRSType GetType() const override { return SRSType::DX11; }


protected:
	//
	std::pair<SColor3, bool> GetClearColor(const SAppFeaturesMap& features);
	//
	void OnTintChanged(SColor3 globalTint);
	//
	void OnWorldScaleChanged(SVector2 worldScale);
	//
	void OnCameraViewChanged(const SCamera& camera);


protected:
	//
	SColoredSpriteRendererDX11 coloredSpriteRendererDX11;


protected:
	//
	IWorld* world{};
	//
	SDXShaderManager shaderManager;
	//
	SConstantBuffersDX11 constantBuffers;
	//
	std::unordered_map<std::string, SShaderDataDX11> shaders;
	//
	ComPtr<IDXGISwapChain> swapChain;
	//
	ComPtr<ID3D11Device> d3dDevice;
	//
	ComPtr<ID3D11DeviceContext> deviceContext;
	//
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	//
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	//
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	//
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	//
	ComPtr<ID3D11BlendState> blendState;
	//
	ComPtr<ID3D11RasterizerState> rasterizerState;
	//
	SSize2 renderSystemSize{};
	//
	bool bNeedDebugTrace = false;
	//
	int maxRefreshRate = 0;

};
