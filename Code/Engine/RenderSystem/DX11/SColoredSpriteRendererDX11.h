/***************************************************************************
* SColoredSpriteRendererDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


struct DX11COLOREDSPRITEINSTANCE
{
	SVector3 pos;
	float    rotation;
	SVector2 scale;
	SColor4F colors[4];
};

/***************************************************************************
* Colored sprite renderer
*/
class SColoredSpriteRendererDX11 : public IVisualRenderer
{
public:
	//
	SColoredSpriteRendererDX11(class SRenderSystemDX11& renderSystem);
	//
	bool CheckShaderName(const std::string& shaderName);


public: // IVisual2DRenderer interface implementation
	//
	virtual ~SColoredSpriteRendererDX11() override;
	//
	virtual void Setup(IVisualRenderer::SShaderData& shaderData) override;
	//
	virtual void Render() override;
	//
	virtual void Shutdown() override;


protected:
	//
	void RenderBatch();


protected:
	//
	SRenderSystemDX11& renderSystemDX11;
	//
	struct ID3D11DeviceContext* d3dDeviceContext;
	//
	struct ID3D11Buffer* spriteVertexBuffer;
	//
	struct ID3D11Buffer* spriteIndexBuffer;
	//
	ComPtr<ID3D11Buffer> instanceBuffer;
	//
	std::string shaderName;
	//
	std::uint32_t numSprites;
	//
	std::uint32_t batchesRendered;
	//
	std::vector<DX11COLOREDSPRITEINSTANCE> batchData;

};
