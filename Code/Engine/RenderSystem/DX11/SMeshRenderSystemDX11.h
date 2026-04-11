/***************************************************************************
* SMeshRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"


/***************************************************************************
* Static mesh render system
*/
class SMeshRenderSystemDX11 : public SUncopyable
{
public:
	//
	SMeshRenderSystemDX11(class IRenderSystemDX11& renderSystem);
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
	IRenderSystemDX11& renderSystemDX11;
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
