/***************************************************************************
* STextRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "Application/SLocalizationInterface.h"


/***************************************************************************
* Text render system
*/
class STextRenderSystemDX11 : public SUncopyable
{
public:
	//
	STextRenderSystemDX11(class IRenderSystemDX11& renderSystem);
	//
	~STextRenderSystemDX11();
	//
	void Shutdown();
	//
	bool CheckShaderName(const std::string& shaderName);
	//
	void Setup(IRenderSystem::SShaderData& shaderData);
	//
	void Render(float deltaSeconds, float gameTime);


protected:
	//
	void RenderBatch();


protected:
	//
	class IRenderSystemDX11& renderSystemDX11;
	//
	struct ID3D11Buffer* spriteVertexBuffer;
	//
	struct ID3D11Buffer* spriteIndexBuffer;
	//
	ComPtr<struct ID3D11Buffer> instanceBuffer;
	//
	std::string shaderName;


protected:
	//
	std::vector<DX11TEXTGLYPHINSTANCE> batchData;
	//
	ID3D11ShaderResourceView* cachedTexView;
	//
	SSize2 cachedTexSize;
	//
	STexID cachedTexId;
	//
	std::uint32_t numGlyphs;
	//
	std::uint32_t batchesRendered;
	//
	float glyphOffset;

};
