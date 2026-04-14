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
	SVector3 position;
};

/** Colored sprite */
struct DX11COLOREDSPRITEINSTANCE
{
	SVector3 position;
	float    rotation;
	SVector2 scale;
	SColor4F colors[4];
};

/** Text glyph data */
struct DX11TEXTGLYPHINSTANCE
{
	SVector3 position;
	SVector2 scale;
	SColor4F color;
	SVector2 uvs[4];
};

/** Mesh instance data */
struct DX11MESHINSTANCE
{
	SVector3 position;
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

/** DirectX 11 cubemap data */
struct SCubemapDataDX11 : public STextureBase
{
	ComPtr<ID3D11Resource> texture;
	ComPtr<ID3D11ShaderResourceView> view;
	std::uint32_t maxMipLevels = 1u;
};

/** DirectX 11 texture data */
struct STextureDataDX11 : public STextureBase
{
	ComPtr<ID3D11Resource> texture;
	ComPtr<ID3D11ShaderResourceView> view;
	SSize2 texSize{};
};

/** DirectX 11 mesh data */
struct SMeshDataDX11 : public SMeshBase
{
	std::vector<SMeshMaterial> materials;
	//
	ComPtr<ID3D11Buffer> vb;
	//
	ComPtr<ID3D11Buffer> ib;
	//
	DXGI_FORMAT ibFormat;
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
	virtual std::uint32_t GetCubemapMaxMipLevel(ECubemapType type) const noexcept = 0;
	//
	virtual std::pair<ID3D11ShaderResourceView*, SSize2> FindTexture(STexID id) const = 0;
	//
	virtual bool FindMesh(SMeshID id, DXGI_FORMAT* outFormat, std::vector<SMeshMaterial>* outMaterials,
		ID3D11Buffer** outVB, ID3D11Buffer** outIB) const = 0;
	//
	virtual class SConstantBuffersDX11& GetConstantBuffers() noexcept = 0;
	//
	virtual class STextureManagerWindows& GetTextureManager() noexcept = 0;
	//
	virtual ID3D11DeviceContext* GetDeviceContext() const noexcept = 0;
	//
	virtual ID3D11Device* GetDevice() const noexcept = 0;
	//
	virtual IWorld* GetWorld() const noexcept = 0;
	//
	virtual bool IsNeedDebugTrace() const noexcept = 0;

};
