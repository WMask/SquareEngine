/***************************************************************************
* SMeshManagerDX11.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "Core/ThreadPool/Fifo4.h"
#include "Core/SUtils.h"
#include "World/SWorldInterface.h"
#include "RenderSystem/SRenderSystemTypes.h"

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <map>

using Microsoft::WRL::ComPtr;


/**
* Mesh manager */
class SMeshManagerDX11
{
public:
	SMeshManagerDX11() : threadPool(nullptr), renderSystem(nullptr) {}
	//
	~SMeshManagerDX11();
	//
	void Init(IThreadPool* threadPool, class SRenderSystemDX11* renderSystem);
	//
	void Shutdown();
	//
	void Update(ID3D11Device* d3dDevice);
	//
	void LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate);
	//
	void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate);
	//
	bool FindMesh(SMeshID id, std::vector<SMaterial>* outMaterials, ID3D11Buffer** outVB, ID3D11Buffer** outIB) const;
	/**
	* If world is not null - only unused meshes removed. If null - all meshes removed. */
	void ClearCache(IWorld* world);
	//
	inline std::uint32_t GetNumTextures() const { return meshesCache.size(); }


protected:
	//
	struct SMeshData
	{
		// generated from fbx path
		SMeshID id{};
		//
		std::vector<SMesh> meshes;
		//
		std::vector<SMeshInstance> instances;
	};
	//
	struct SMeshDataDX11
	{
		std::vector<SMaterial> materials;
		//
		ComPtr<ID3D11Buffer> vb;
		//
		ComPtr<ID3D11Buffer> ib;
	};
	//
	bool LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData);
	//
	bool CreateMesh(ID3D11Device* device, const SMesh& meshData, SMeshDataDX11& outMesh);
	//
	void CheckLoadFinished(const SMeshData& meshData);
	// after any meshesCache changes
	void UpdateCacheIds();


protected:
	//
	using TInstancesDelegatesCache = std::list<std::pair<std::filesystem::path, OnMeshInstancesLoadedDelegate>>;
	//
	using TPreloadDelegatesCache = std::list<std::pair<std::filesystem::path, OnMeshesLoadedDelegate>>;
	//
	using TMeshesCacheIds = std::shared_ptr<std::forward_list<SMeshID>>;
	//
	using TCircularFIFOTextureQueue = Fifo4<SMeshData>;
	//
	std::shared_ptr<TCircularFIFOTextureQueue> loadedMeshes;
	//
	std::unordered_map<SMeshID, SMeshDataDX11> meshesCache;
	//
	std::atomic<TMeshesCacheIds> meshesCacheIds;
	//
	TInstancesDelegatesCache instancesDelegatesCache;
	//
	TPreloadDelegatesCache preloadDelegatesCache;
	//
	class SRenderSystemDX11* renderSystem;
	//
	IThreadPool* threadPool;

};
