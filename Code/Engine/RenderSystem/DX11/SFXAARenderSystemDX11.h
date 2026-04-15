/***************************************************************************
* SFXAARenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


struct DX11RENDERTARGETVERTEX
{
	SVector3 position;
	SVector2 uv;
};


/***************************************************************************
* FXAA render system
*/
class SFXAARenderSystemDX11 : public SUncopyable
{
public:
	//
	SFXAARenderSystemDX11(class IRenderSystemDX11& renderSystem);
	//
	~SFXAARenderSystemDX11();
	//
	void Shutdown();
	//
	bool CheckShaderName(const std::string& shaderName);
	//
	void Setup(IRenderSystem::SShaderData& shaderData);
	//
	void Render(ID3D11ShaderResourceView* sceneSRV, const SSize2& sceneSize);


protected:
	//
	class IRenderSystemDX11& renderSystemDX11;
	//
	ComPtr<ID3D11Buffer> spriteVertexBuffer;
	//
	ComPtr<ID3D11Buffer> spriteIndexBuffer;
	//
	std::string shaderName;

};
