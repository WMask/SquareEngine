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
	void LoadSkeletalMesh(const std::filesystem::path& path, OnSkeletalMeshLoadedDelegate delegate);
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
	bool LoadSkeletalMeshData(const std::filesystem::path& path, SSkeletalMeshData& meshData);
	//
	void CheckLoadFinished(const SMeshData& meshData, const SSkeletalMeshData& skMeshData);


protected:
	//
	using TSkeletalMeshDelegatesCache = std::list<std::pair<SMeshID, OnSkeletalMeshLoadedDelegate>>;
	//
	using TInstancesDelegatesCache = std::list<std::pair<SMeshID, OnMeshInstancesLoadedDelegate>>;
	//
	using TPreloadDelegatesCache = std::list<std::pair<SMeshID, OnMeshesLoadedDelegate>>;
	//
	using TMeshesCache = std::unordered_map<SMeshID, std::shared_ptr<SMeshBase>>;
	//
	using TSkeletalMeshQueue = std::queue<SSkeletalMeshData>;
	//
	using TMeshQueue = std::queue<SMeshData>;
	//
	TSkeletalMeshDelegatesCache skeletalDelegatesCache;
	//
	TInstancesDelegatesCache instancesDelegatesCache;
	//
	TPreloadDelegatesCache preloadDelegatesCache;
	//
	TMeshesCache meshesCache;
	//
	TMeshQueue loadedMeshes;
	//
	TSkeletalMeshQueue loadedSkeletalMeshes;
	//
	IMeshLifetime* meshLifetime;
	//
	IThreadPool* threadPool;
	//
	std::mutex sync;

};
