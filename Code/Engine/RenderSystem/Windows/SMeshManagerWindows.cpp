/***************************************************************************
* SMeshManagerWindows.h
*/

#ifdef WIN32

#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "RenderSystem/Windows/SMeshManagerWindows.h"
#include "RenderSystem/Windows/STextureManagerWindows.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <vector>
#include <ranges>
#include <cmath>


namespace SConst
{
    constexpr std::uint32_t MaxMeshesPerFrame = 4u;
}

SMeshManagerWindows::~SMeshManagerWindows()
{
    Shutdown();
}

void SMeshManagerWindows::Init(IThreadPool* inThreadPool, IMeshLifetime* inMeshLifetime)
{
    threadPool = inThreadPool;
    meshLifetime = inMeshLifetime;
    meshesCache.reserve(128);
}

void SMeshManagerWindows::Shutdown()
{
    if (!meshLifetime) return;

    ClearCache(nullptr);
    threadPool = nullptr;
    meshLifetime = nullptr;

    {
        TLockGuard guard(sync);
        TMeshQueue meshes;
        TAnimationQueue animations;
        TSkeletalMeshQueue skeletalMeshes;
        loadedMeshes.swap(meshes);
        loadedAnimations.swap(animations);
        loadedSkeletalMeshes.swap(skeletalMeshes);
    }
}

const SBakedSkeletalAnimation* SMeshManagerWindows::FindAnimation(SAnimID id) const
{
    auto it = animsCache.find(id);
    if (it != animsCache.end())
    {
        return &it->second;
    }

    return nullptr;
}

SMeshBase* SMeshManagerWindows::FindMesh(SMeshID id) const
{
    auto it = meshesCache.find(id);
    if (it != meshesCache.end())
    {
        return it->second.get();
    }

    return nullptr;
}

bool SMeshManagerWindows::RemoveAnimation(SAnimID id)
{
    if (animsCache.contains(id))
    {
        animsCache.erase(id);
        return true;
    }

    return false;
}

bool SMeshManagerWindows::RemoveMesh(SMeshID id)
{
    if (meshesCache.contains(id))
    {
        meshesCache.erase(id);
        return true;
    }

    return false;
}

void SMeshManagerWindows::Update()
{
    if (!meshLifetime) return;

    S_TRY

    for (std::uint32_t i = 0; i < SConst::MaxMeshesPerFrame; i++)
    {
        SMeshData meshData{};
        SSkeletalAnimData animData{};
        SSkeletalMeshData skMeshData{};
        if (!loadedMeshes.empty() ||
            !loadedSkeletalMeshes.empty() ||
            !loadedAnimations.empty())
        {
            TLockGuard guard(sync);
            // read in game thread space
            if (!loadedMeshes.empty())
            {
                meshData = loadedMeshes.front();
                loadedMeshes.pop();
            }

            if (!loadedSkeletalMeshes.empty())
            {
                skMeshData = loadedSkeletalMeshes.front();
                loadedSkeletalMeshes.pop();
            }

            if (!loadedAnimations.empty())
            {
                animData = loadedAnimations.front();
                loadedAnimations.pop();
            }
        }

        if (!meshData.meshes.empty() && !meshData.bLoadFailed)
        {
            for (auto& data : meshData.meshes)
            {
                auto mesh = meshLifetime->CreateMesh(data);
                if (!mesh)
                {
                    throw std::exception("Cannot create mesh");
                }

                meshesCache.emplace(data.id, mesh);
            }
        }

        if (!skMeshData.mesh.vertices.empty() && !skMeshData.bLoadFailed)
        {
            auto mesh = meshLifetime->CreateSkeletalMesh(skMeshData.mesh);
            if (!mesh)
            {
                throw std::exception("Cannot create skeletal mesh");
            }

            meshesCache.emplace(skMeshData.mesh.id, mesh);
        }

        if (!animData.anim.frames.empty() && !animData.bLoadFailed)
        {
            animsCache.emplace(animData.anim.id, animData.anim);
        }

        if (!preloadDelegatesCache.empty() ||
            !instancesDelegatesCache.empty() ||
            !skeletalDelegatesCache.empty() ||
            !animationDelegatesCache.empty())
        {
            CheckLoadFinished(meshData, skMeshData, animData);
        }

        if (meshData.meshes.empty() &&
            meshData.instances.empty() &&
            skMeshData.mesh.vertices.empty() &&
            animData.anim.frames.empty())
        {
            break;
        }
    }

    S_CATCH{ S_THROW("SMeshManagerWindows::Update()"); }
}

void SMeshManagerWindows::LoadStaticMeshInstances(const std::filesystem::path& path,
    SGroupID groupId, OnMeshInstancesLoadedDelegate delegate)
{
    DebugMsg("[%s] SMeshManagerWindows::LoadStaticMeshInstances(): begin loading instances from '%s'\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());

    SMeshID meshId = ResourceID<SMeshID>(path.string());
    instancesDelegatesCache.emplace_back(meshId, delegate);

    auto LoadInstancesTask = [this, path, meshId, groupId]()
    {
        SMeshData meshesData{};
        meshesData.id = meshId;
        meshesData.path = path;
        if (!LoadMeshData(path, groupId, meshesData))
        {
            meshesData.meshes.clear();
            meshesData.instances.clear();
            meshesData.bLoadFailed = true;
            DebugMsg("[%s] SMeshManagerWindows::LoadStaticMeshInstances(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }

        // write in thread pool space
        TLockGuard guard(sync);
        loadedMeshes.push(meshesData);
    };

    // send task to thread pool
    threadPool->AddTask(LoadInstancesTask, "Load mesh instances");
}

void SMeshManagerWindows::PreloadStaticMeshes(const std::filesystem::path& path, OnMeshFinishedDelegate delegate)
{
    DebugMsg("[%s] SMeshManagerWindows::PreloadStaticMeshes(): begin loading meshes from '%s'\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());

    SMeshID meshId = ResourceID<SMeshID>(path.string());
    preloadDelegatesCache.emplace_back(meshId, delegate);

    auto PreloadMeshesTask = [this, path, meshId]()
    {
        SMeshData meshesData{};
        // save id even with failed data to call delegate
        meshesData.id = meshId;
        meshesData.path = path;
        if (!LoadMeshData(path, 0u, meshesData))
        {
            meshesData.meshes.clear();
            meshesData.instances.clear();
            meshesData.bLoadFailed = true;
            DebugMsg("[%s] SMeshManagerWindows::PreloadStaticMeshes(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }

        // write in thread pool space
        TLockGuard guard(sync);
        loadedMeshes.push(std::move(meshesData));
    };

    // send task to thread pool
    threadPool->AddTask(PreloadMeshesTask, "Preload meshes");
}

void SMeshManagerWindows::LoadSkeletalMesh(const std::filesystem::path& path,
    OnSkeletalMeshLoadedDelegate delegate)
{
    DebugMsg("[%s] SMeshManagerWindows::LoadSkeletalMesh(): begin loading mesh from '%s'\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());

    SMeshID meshId = ResourceID<SMeshID>(path.string());
    skeletalDelegatesCache.emplace_back(meshId, delegate);

    auto LoadSkeletalMeshTask = [this, path, meshId]()
    {
        SSkeletalMeshData meshData{};
        // save id even with failed data to call delegate
        meshData.id = meshId;
        if (!LoadSkeletalMeshData(path, meshData))
        {
            ClearMesh(meshData.mesh);
            meshData.bLoadFailed = true;
            DebugMsg("[%s] SMeshManagerWindows::LoadSkeletalMesh(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }

        // write in thread pool space
        TLockGuard guard(sync);
        loadedSkeletalMeshes.push(std::move(meshData));
    };

    // send task to thread pool
    threadPool->AddTask(LoadSkeletalMeshTask, "Load skeletal mesh");
}

void SMeshManagerWindows::PreloadAnimations(const SPathList& paths, SMeshID meshId,
    OnAnimationsLoadedDelegate delegate)
{
    DebugMsg("[%s] SMeshManagerWindows::PreloadAnimations(): begin loading %u animations\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), paths.size());

    // transform paths to ids
    auto viewIds = paths | std::views::transform([](const auto& path) {
        return ResourceID<SAnimID>(MakeAnimationName(path));
    });
    TAnimIDList ids(viewIds.begin(), viewIds.end());

    // cache delegate and ids
    animationDelegatesCache.emplace_back(ids, delegate);

    auto PreloadAnimsTask = [this, paths, meshId, ids]()
    {
        std::vector<SSkeletalAnimData> anims;
        anims.reserve(paths.size());

        for (auto i = 0; i < paths.size(); i++)
        {
            auto& path = paths[i];
            auto animId = ids[i];

            SSkeletalAnimData animData{};
            // save id even with failed data to call delegate
            animData.id = animId;
            if (LoadAnimationData(path, meshId, animData))
            {
                animData.anim.meshId = meshId;
            }
            else
            {
                animData.bLoadFailed = true;
                animData.anim.frames.clear();
                DebugMsg("[%s] SMeshManagerWindows::PreloadAnimations(): cannot load '%s'\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
            }

            anims.push_back(animData);
        }

        // write in thread pool space
        TLockGuard guard(sync);
        for (auto& anim : anims)
        {
            loadedAnimations.push(std::move(anim));
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadAnimsTask, "Preload animations");

}

bool SMeshManagerWindows::LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData)
{
    try
    {
        auto keysView = meshesCache | std::views::keys;
        std::forward_list<SMeshID> ids(keysView.begin(), keysView.end());

        if (LoadFbxStaticMeshes(path, groupId, ids, meshesData.meshes, meshesData.instances)) return true;
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
    }

    return false;
}

bool SMeshManagerWindows::LoadSkeletalMeshData(const std::filesystem::path& path, SSkeletalMeshData& meshData)
{
    try
    {
        if (LoadFbxSkeletalMesh(path, meshData.mesh)) return true;
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadSkeletalMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
    }

    return false;
}

bool SMeshManagerWindows::LoadAnimationData(const std::filesystem::path& path, SMeshID id, SSkeletalAnimData& animData)
{
    try
    {
        TBonesMap bones;
        meshLifetime->GetBones(id, bones);
        auto meshIt = meshesCache.find(id);
        if (!meshLifetime->GetBones(id, bones) || bones.empty())
        {
            DebugMsg("[%s] SMeshManagerWindows::LoadAnimationData(): Cannot get bones for mesh id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), id);
            return false;
        }

        if (LoadFbxBakedAnimation(path, bones, animData.anim)) return true;
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadAnimationData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
    }

    return false;
}

void SMeshManagerWindows::CheckLoadFinished(const SMeshData& meshData, const SSkeletalMeshData& skMeshData,
    const SSkeletalAnimData& animData)
{
    // static meshes
    if (!meshData.meshes.empty() || !meshData.instances.empty())
    {
        std::forward_list<TInstancesDelegatesCache::const_iterator> eraseList1;
        std::forward_list<TPreloadMeshesDelegatesCache::const_iterator> eraseList2;

        for (auto it = instancesDelegatesCache.begin(); it != instancesDelegatesCache.end(); ++it)
        {
            if (it->first == meshData.id)
            {
                eraseList1.push_front(it);

                if (it->second)
                {
                    // call delegate
                    it->second(!meshData.bLoadFailed, meshData.instances, *this);
                }
            }
        }

        for (auto it = preloadDelegatesCache.begin(); it != preloadDelegatesCache.end(); ++it)
        {
            if (it->first == meshData.id)
            {
                eraseList2.push_front(it);

                if (it->second)
                {
                    // call delegate
                    it->second(!meshData.bLoadFailed, *this);
                }
            }
        }

        for (auto eraseIt : eraseList1)
        {
            instancesDelegatesCache.erase(eraseIt);
        }

        for (auto eraseIt : eraseList2)
        {
            preloadDelegatesCache.erase(eraseIt);
        }

        if (!meshData.bLoadFailed)
        {
            if (!meshData.meshes.empty())
            {
                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): static meshes from '%s', id=%u created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.path.string().c_str(), meshData.id);
            }

            if (!meshData.instances.empty())
            {
                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): mesh instances from '%s', id=%u loaded\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.path.string().c_str(), meshData.id);
            }
        }
    }

    // skeletal meshes
    if (!skMeshData.mesh.vertices.empty())
    {
        std::forward_list<TSkeletalMeshDelegatesCache::const_iterator> eraseList;

        for (auto it = skeletalDelegatesCache.begin(); it != skeletalDelegatesCache.end(); ++it)
        {
            if (it->first == skMeshData.id)
            {
                eraseList.push_front(it);

                if (it->second)
                {
                    // call delegate
                    it->second(!skMeshData.bLoadFailed, skMeshData.mesh.id, skMeshData.mesh.transform, *this);
                }
            }
        }

        for (auto eraseIt : eraseList)
        {
            skeletalDelegatesCache.erase(eraseIt);
        }

        if (!skMeshData.bLoadFailed)
        {
            DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): skeletal mesh '%s', id=%u created and added to cache\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), skMeshData.mesh.name.c_str(), skMeshData.mesh.id);
        }
    }

    // animations
    if (!animData.anim.frames.empty())
    {
        std::forward_list<TPreloadAnimsDelegatesCache::const_iterator> eraseList;

        for (auto it = animationDelegatesCache.begin(); it != animationDelegatesCache.end(); ++it)
        {
            bool bAllLoaded = true;
            bool bLoadFailed = false;

            for (auto id : it->first)
            {
                if (animData.id == id &&
                    animData.bLoadFailed)
                {
                    bLoadFailed = true;
                    break;
                }

                if (!animsCache.contains(id))
                {
                    bAllLoaded = false;
                    break;
                }
            }

            if (bLoadFailed)
            {
                eraseList.push_front(it);

                if (it->second)
                {
                    // fail all if one of the textures failed
                    it->second(false, it->first, *this);
                }
            }
            else if (bAllLoaded)
            {
                eraseList.push_front(it);

                if (it->second)
                {
                    // call delegate
                    it->second(true, it->first, *this);
                }
            }
        }

        for (auto eraseIt : eraseList)
        {
            animationDelegatesCache.erase(eraseIt);
        }

        DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): animation '%s', id=%u loaded and added to cache\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), animData.anim.name.c_str(), animData.anim.id);
    }
}

void SMeshManagerWindows::ClearCache(IWorld* world)
{
    S_TRY

    if (world)
    {
        std::set<STexID> aliveMeshList;
        const auto& registry = world->GetEntities();
        const auto& view = registry.view<SStaticMeshComponent>();

        view.each([&aliveMeshList](const SStaticMeshComponent& meshComponent)
        {
            aliveMeshList.insert(meshComponent.id);
        });

        std::set<STexID> eraseTexList;
        for (auto tex : meshesCache)
        {
            if (!aliveMeshList.contains(tex.first))
            {
                eraseTexList.insert(tex.first);
            }
        }

        for (auto entry : eraseTexList)
        {
            meshesCache.erase(entry);
        }
    }
    else
    {
        meshesCache.clear();
    }

    S_CATCH{ S_THROW("SMeshManagerWindows::ClearCache()") }
}

#endif // WIN32
