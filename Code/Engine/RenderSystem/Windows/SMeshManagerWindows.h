/***************************************************************************
* SMeshManagerWindows.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "RenderSystem/Windows/SUtilsWindows.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "World/SWorldInterface.h"
#include "Core/SUtils.h"

#include <cstdint>
#include <queue>
#include <map>


/***************************************************************************
* Mesh manager
*/
class SMeshManagerWindows
{
public:
	SMeshManagerWindows() : threadPool(nullptr), meshLifetime(nullptr) {}
	//
	~SMeshManagerWindows();
	//
	void Init(IThreadPool* threadPool, IMeshLifetime* inMeshLifetime);
	//
	void Shutdown();
	//
	void Update();
	//
	void LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate);
	//
	void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate);
	//
	SMeshBase* FindMesh(SMeshID id) const;
	//
	bool RemoveMesh(SMeshID id);
	/**
	* If world is not null - only unused meshes removed. If null - all meshes removed. */
	void ClearCache(IWorld* world);
	//
	inline std::uint32_t GetNumMeshes() const { return meshesCache.size(); }


protected:
	//
	bool LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData);
	//
	void CheckLoadFinished(const SMeshData& meshData);
	// Update after any meshesCache changes. Used to skip already loaded meshes in LoadFbxStaticMeshes
	void UpdateCacheIds();


protected:
	//
	using TInstancesDelegatesCache = std::list<std::pair<std::filesystem::path, OnMeshInstancesLoadedDelegate>>;
	//
	using TPreloadDelegatesCache = std::list<std::pair<std::filesystem::path, OnMeshesLoadedDelegate>>;
	//
	using TMeshesCache = std::unordered_map<SMeshID, std::shared_ptr<SMeshBase>>;
	//
	using TMeshesCacheIds = std::forward_list<SMeshID>;
	//
	using TMeshQueue = std::queue<SMeshData>;
	//
	TInstancesDelegatesCache instancesDelegatesCache;
	//
	TPreloadDelegatesCache preloadDelegatesCache;
	//
	TMeshesCacheIds meshesCacheIds;
	//
	TMeshesCache meshesCache;
	//
	TMeshQueue loadedMeshes;
	//
	IMeshLifetime* meshLifetime;
	//
	IThreadPool* threadPool;
	//
	std::mutex sync;

};
