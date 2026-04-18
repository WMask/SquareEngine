/***************************************************************************
* SSkeletalMeshRenderSystemDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"


/***************************************************************************
* Skeletal mesh render system
*/
class SSkeletalMeshRenderSystemDX11 : public SUncopyable
{
public:
	//
	SSkeletalMeshRenderSystemDX11(class IRenderSystemDX11& renderSystem);
	//
	~SSkeletalMeshRenderSystemDX11();
	//
	void Shutdown();
	//
	bool CheckShaderName(const std::string& shaderName);
	//
	void Setup(IRenderSystem::SShaderData& shaderData);
	//
	void Render(float deltaSeconds);


protected:
	//
	void RenderSkeletalMesh(const struct SShaderDataDX11* shader,
		const struct SSkeletalMeshComponent& meshComponent,
		const STransform& transform);


protected:
	//
	IRenderSystemDX11& renderSystemDX11;
	//
	std::string shaderName;
	//
	SBakedSkeletalAnimation anim;


protected:
	//
	std::vector<SMeshMaterial> cachedMaterials;
	//
	SMaterialFlagsBuffer cachedMaterialFlags;
	//
	DXGI_FORMAT cachedIbFormat;
	//
	ID3D11Buffer* cachedVB;
	//
	ID3D11Buffer* cachedIB;
	//
	SMeshID cachedMeshId;
	//
	std::uint32_t meshesRendered;

};
