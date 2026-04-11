/***************************************************************************
* SColoredSpriteRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"


/***************************************************************************
* Colored sprite render system
*/
class SColoredSpriteRenderSystemDX11 : public SUncopyable
{
public:
	//
	SColoredSpriteRenderSystemDX11(class IRenderSystemDX11& renderSystem);
	//
	~SColoredSpriteRenderSystemDX11();
	//
	bool CheckShaderName(const std::string& shaderName);
	//
	void Setup(IRenderSystem::SShaderData& shaderData);
	//
	void Render(float deltaSeconds, float gameTime);
	//
	void Shutdown();


protected:
	//
	void RenderBatch();


protected:
	//
	IRenderSystemDX11& renderSystemDX11;
	//
	struct ID3D11Buffer* spriteVertexBuffer;
	//
	struct ID3D11Buffer* spriteIndexBuffer;
	//
	ComPtr<ID3D11Buffer> instanceBuffer;
	//
	std::string shaderName;


protected:
	//
	std::vector<DX11COLOREDSPRITEINSTANCE> batchData;
	//
	std::uint32_t numSprites;
	//
	std::uint32_t batchesRendered;

};
