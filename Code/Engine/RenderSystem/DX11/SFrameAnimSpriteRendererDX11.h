/***************************************************************************
* SFrameAnimSpriteRendererDX11.h
*/

#pragma once

#include "RenderSystem/DX11/STexturedSpriteRendererDX11.h"


/***************************************************************************
* Frame animated sprite render system
*/
class SFrameAnimSpriteRendererDX11 : public STexturedSpriteRendererDX11
{
public:
	//
	SFrameAnimSpriteRendererDX11(class SRenderSystemDX11& renderSystem) : STexturedSpriteRendererDX11(renderSystem) {}
	//
	~SFrameAnimSpriteRendererDX11() {}
	//
	virtual void Render(float deltaSeconds, float gameTime) override;


protected:
	//
	virtual void RenderBatch(struct ID3D11ShaderResourceView* view) override;

};
