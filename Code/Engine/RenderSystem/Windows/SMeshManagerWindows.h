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
	void LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId,
		OnMeshInstancesLoadedDelegate delegate);
	//
	void PreloadStaticMeshes(const std::filesystem::path& path, OnFinishedDelegate delegate);
	//
	void LoadSkeletalMesh(const std::filesystem::path& path, OnSkeletalMeshLoadedDelegate delegate);
	//
	void PreloadAnimations(const SPathList& paths, SMeshID id, OnAnimationsLoadedDelegate delegate);
	//
	const SBakedSkeletalAnimation* FindAnimation(SAnimID id) const;
	//
	SMeshBase* FindMesh(SMeshID id) const;
	//
	bool RemoveAnimation(SAnimID id);
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
	bool LoadAnimationData(const std::filesystem::path& path, SMeshID id, SSkeletalAnimData& animData);
	//
	void CheckLoadFinished(const SMeshData& meshData, const SSkeletalMeshData& skMeshData,
		const SSkeletalAnimData& animData);


protected:
	//
	using TAnimIDList = std::vector<SAnimID>;
	//
	using TSkeletalMeshDelegatesCache = std::list<std::pair<SMeshID, OnSkeletalMeshLoadedDelegate>>;
	//
	using TInstancesDelegatesCache = std::list<std::pair<SMeshID, OnMeshInstancesLoadedDelegate>>;
	//
	using TPreloadAnimsDelegatesCache = std::list<std::pair<TAnimIDList, OnAnimationsLoadedDelegate>>;
	//
	using TPreloadMeshesDelegatesCache = std::list<std::pair<SMeshID, OnFinishedDelegate>>;
	//
	using TAnimsCache = std::unordered_map<SAnimID, SBakedSkeletalAnimation>;
	//
	using TMeshesCache = std::unordered_map<SMeshID, std::shared_ptr<SMeshBase>>;
	//
	using TSkeletalMeshQueue = std::queue<SSkeletalMeshData>;
	//
	using TAnimationQueue = std::queue<SSkeletalAnimData>;
	//
	using TMeshQueue = std::queue<SMeshData>;
	//
	TSkeletalMeshDelegatesCache skeletalDelegatesCache;
	//
	TPreloadAnimsDelegatesCache animationDelegatesCache;
	//
	TInstancesDelegatesCache instancesDelegatesCache;
	//
	TPreloadMeshesDelegatesCache preloadDelegatesCache;
	//
	TMeshesCache meshesCache;
	//
	TAnimsCache animsCache;
	//
	TMeshQueue loadedMeshes;
	//
	TAnimationQueue loadedAnimations;
	//
	TSkeletalMeshQueue loadedSkeletalMeshes;
	//
	IMeshLifetime* meshLifetime;
	//
	IThreadPool* threadPool;
	//
	std::mutex sync;

};
