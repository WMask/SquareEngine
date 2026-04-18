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
class SMeshManagerWindows : public IMeshManager
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


public:// IMeshManager interface implementation
	//
	virtual void LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId,
		OnMeshInstancesLoadedDelegate delegate) override;
	//
	virtual void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshFinishedDelegate delegate) override;
	//
	virtual void LoadSkeletalMesh(const std::filesystem::path& path, OnSkeletalMeshLoadedDelegate delegate) override;
	//
	virtual void PreloadAnimations(const SPathList& paths, SMeshID id, OnAnimationsLoadedDelegate delegate) override;


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
	using TPreloadMeshesDelegatesCache = std::list<std::pair<SMeshID, OnMeshFinishedDelegate>>;
	//
	using TPreloadAnimsDelegatesCache = std::list<std::pair<TAnimIDList, OnAnimationsLoadedDelegate>>;
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
