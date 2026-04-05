/***************************************************************************
* SMeshRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


struct DX11MESHINSTANCE
{
	SVector3 pos;
	SQuat    rotation;
	SVector3 scale;
	SColor3F tint;
};

/***************************************************************************
* Static mesh render system
*/
class SMeshRenderSystemDX11 : public SUncopyable
{
public:
	//
	SMeshRenderSystemDX11(class SRenderSystemDX11& renderSystem);
	//
	~SMeshRenderSystemDX11();
	//
	void Shutdown();
	//
	bool CheckShaderName(const std::string& shaderName);
	//
	void Setup(IRenderSystem::SShaderData& shaderData);
	//
	virtual void Render(float deltaSeconds);


protected:
	//
	void RenderBatch(const struct SShaderDataDX11* shader);


protected:
	//
	class SRenderSystemDX11& renderSystemDX11;
	//
	ID3D11DeviceContext* d3dDeviceContext;
	//
	ComPtr<ID3D11Buffer> instanceBuffer;
	//
	std::string shaderName;


protected:
	//
	std::vector<DX11MESHINSTANCE> batchData;
	//
	std::vector<SMaterial> cachedMaterials;
	//
	SMaterialBuffer cachedMaterialFlags;
	//
	ID3D11Buffer* cachedVB;
	//
	ID3D11Buffer* cachedIB;
	//
	SMeshID cachedMeshId;
	//
	std::uint32_t cachedNumIndices;
	//
	std::uint32_t numMeshInstances;
	//
	std::uint32_t batchesRendered;

};
