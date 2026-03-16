/***************************************************************************
* SRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"

#pragma warning(disable : 4251)


/***************************************************************************
* DirectX 11 render system
*/
class S_RENDERSYSTEM_API SRenderSystemDX11 : public SRenderSystemBase
{
public:
	//
	SRenderSystemDX11();
	/**
	* Virtual destructor */
	virtual ~SRenderSystemDX11();


public:// IRenderSystem interface implementation
	//
	virtual void Create(void* windowHandle, const SAppFeaturesMap& features, SAppMode mode, const SAppContext& context) override;
	//
	virtual void Shutdown() override;
	//
	virtual void Update(float deltaSeconds, const SAppContext& context) override;
	//
	virtual void Render(const SAppContext& context) override;
	//
	virtual SRSType GetType() const override { return SRSType::DX11; }


protected:// SRenderSystemBase interface implementation
	//
	virtual void Render(const class IVisual* visual, const SAppContext& context) override;

};
