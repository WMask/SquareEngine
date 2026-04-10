/***************************************************************************
* SFrameAnimSpriteRenderSystemDX11.h
*/

#pragma once

#include "RenderSystem/DX11/STexturedSpriteRenderSystemDX11.h"


/***************************************************************************
* Frame animated sprite render system
*/
class SFrameAnimSpriteRenderSystemDX11 : public STexturedSpriteRenderSystemDX11
{
public:
	//
	SFrameAnimSpriteRenderSystemDX11(class IRenderSystemDX11& renderSystem) : STexturedSpriteRenderSystemDX11(renderSystem) {}
	//
	~SFrameAnimSpriteRenderSystemDX11() {}
	//
	virtual void Render(float deltaSeconds, float gameTime) override;


protected:
	//
	void RenderBatch();


protected:
	//
	SSize2 cachedTexSize;

};
