/***************************************************************************
* SRenderSystemTypesDX11.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


/** Sprite vertex data */
struct DX11SPRITEVERTEX
{
	SVector3 pos;
};

/** Text glyph data */
struct DX11TEXTGLYPHINSTANCE
{
	SVector3 pos;
	SVector2 scale;
	SColor4F color;
	SVector2 uvs[4];
};

/** Mesh instance data */
struct DX11MESHINSTANCE
{
	SVector3 pos;
	SQuat    rotation;
	SVector3 scale;
	SColor3F tint;
};

/** DirectX 11 shader data */
struct SShaderDataDX11 : public IRenderSystem::SShaderData
{
	ComPtr<ID3D11VertexShader> vs;
	//
	ComPtr<ID3D11PixelShader> ps;
	//
	ComPtr<ID3D11InputLayout> layout;
	//
	ID3DBlob* vsCode{};
};


/***************************************************************************
* DirectX 11 render system interface
*/
class IRenderSystemDX11 : public IRenderSystemEx
{
public:
	//
	virtual const SShaderDataDX11* FindShader(const std::string& name) const = 0;
	//
	virtual ID3D11ShaderResourceView* FindCubemap(ECubemapType type) const = 0;
	//
	virtual std::pair<ID3D11ShaderResourceView*, SSize2> FindTexture(STexID id) const = 0;
	//
	virtual bool FindMesh(SMeshID id, std::vector<SMaterial>* outMaterials, ID3D11Buffer** outVB, ID3D11Buffer** outIB) const = 0;
	//
	virtual class SConstantBuffersDX11& GetConstantBuffers() noexcept = 0;
	//
	virtual class STextureManagerDX11& GetTextureManager() noexcept = 0;
	//
	virtual ID3D11DeviceContext* GetDeviceContext() const noexcept = 0;
	//
	virtual ID3D11Device* GetDevice() const noexcept = 0;
	//
	virtual IWorld* GetWorld() const noexcept = 0;
	//
	virtual bool IsNeedDebugTrace() const noexcept = 0;

};
