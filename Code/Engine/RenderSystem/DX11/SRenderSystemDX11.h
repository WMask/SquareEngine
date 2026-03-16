/***************************************************************************
* SRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>

using namespace Microsoft::WRL;

#pragma warning(disable : 4251)


/***************************************************************************
* DirectX 11 render system
*/
class S_RENDERSYSTEM_API SRenderSystemDX11 : public SRenderSystemBase
{
public:
	//
	SRenderSystemDX11();


public:// IRenderSystem interface implementation
	//
	virtual ~SRenderSystemDX11() override;
	//
	virtual void Create(void* windowHandle, const SAppFeaturesMap& features, SAppMode mode, const SAppContext& context) override;
	//
	virtual void Shutdown() override;
	//
	virtual void Update(float deltaSeconds, const SAppContext& context) override;
	//
	virtual void Render(const SAppContext& context) override;
	//
	virtual void Clear(class IWorld* world, bool removeRooted = false) override {}
	//
	virtual bool CanRender() const override { return (renderTargetView.Get() != nullptr); }
	//
	virtual void RequestResize(std::int32_t width, std::int32_t height) override {}
	//
	virtual void Resize(std::int32_t width, std::int32_t height, const SAppContext& context) override {}
	//
	virtual void SetMode(SAppMode mode) override {}
	//
	virtual void UpdateCamera(float deltaSeconds, SVector3 newPos, SVector3 newTarget) override {}
	//
	virtual SRSStats GetStats() const override { return SRSStats{}; }
	//
	virtual SRSType GetType() const override { return SRSType::DX11; }


protected:// SRenderSystemBase interface implementation
	//
	virtual void Render(const class IVisual* visual, const SAppContext& context) override;


protected:
	//
	ComPtr<IDXGISwapChain> swapChain;
	//
	ComPtr<ID3D11Device> device;
	//
	ComPtr<ID3D11DeviceContext> deviceContext;
	//
	ComPtr<ID3D11RenderTargetView> renderTargetView;

};
