/***************************************************************************
* STextRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"
#include "Application/SLocalizationInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


struct DX11TEXTGLYPHINSTANCE
{
	SVector3 pos;
	SVector2 scale;
	SColor4F color;
	SVector2 uvs[4];
};

/***************************************************************************
* Text render system
*/
class STextRenderSystemDX11 : public SUncopyable
{
public:
	//
	STextRenderSystemDX11(class SRenderSystemDX11& renderSystem);
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
	class SRenderSystemDX11& renderSystemDX11;
	//
	struct ID3D11DeviceContext* d3dDeviceContext;
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
	const class IWorld* world;
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
