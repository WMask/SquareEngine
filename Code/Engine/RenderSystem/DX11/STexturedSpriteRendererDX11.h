/***************************************************************************
* STexturedSpriteRendererDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


/***************************************************************************
* Textured sprite renderer
*/
class STexturedSpriteRendererDX11 : public IVisualRenderer
{
public:
	//
	STexturedSpriteRendererDX11() {}
	//
	bool CheckShaderName(const std::string& shaderName);


public: // IVisual2DRenderer interface implementation
	//
	virtual ~STexturedSpriteRendererDX11() override;
	//
	virtual void Setup(IRenderSystem& renderSystem, IVisualRenderer::SShaderData& shaderData) override;
	//
	virtual void Render(IRenderSystem& renderSystem) override;
	//
	virtual void Shutdown() override;


protected:
	//
	ComPtr<ID3D11Buffer> instanceBuffer;
	//
	std::string shaderName;

};
