/***************************************************************************
* STexturedSpriteRendererDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


struct DX11TEXTUREDSPRITEINSTANCE
{
	SVector3 pos;
	float    rotation;
	SVector2 scale;
	SColor4F colors[4];
	SVector2 uvs[4];
};

/***************************************************************************
* Textured sprite renderer
*/
class STexturedSpriteRendererDX11 : public IVisualRenderer
{
public:
	//
	STexturedSpriteRendererDX11(class SRenderSystemDX11& renderSystem);
	//
	bool CheckShaderName(const std::string& shaderName);


public: // IVisual2DRenderer interface implementation
	//
	virtual ~STexturedSpriteRendererDX11() override;
	//
	virtual void Setup(IVisualRenderer::SShaderData& shaderData) override;
	//
	virtual void Render() override;
	//
	virtual void Shutdown() override;


protected:
	//
	void RenderBatch(struct ID3D11ShaderResourceView* view);


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
	std::vector<DX11TEXTUREDSPRITEINSTANCE> batchData;
	//
	std::string shaderName;
	//
	std::uint32_t numSprites;
	//
	std::uint32_t batchesRendered;

};
