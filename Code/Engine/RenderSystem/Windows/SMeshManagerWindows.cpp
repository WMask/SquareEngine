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
    static const std::uint32_t MaxMeshesPerFrame = 4u;
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
        TSkeletalMeshQueue skeletalMeshes;
        loadedMeshes.swap(meshes);
        loadedSkeletalMeshes.swap(skeletalMeshes);
    }
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
        bool bBreak = false;

        SMeshData meshData;
        SSkeletalMeshData skMeshData;
        {
            // read in game thread space
            TLockGuard guard(sync);

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
        }

        if (!meshData.meshes.empty() || !meshData.instances.empty())
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
        else if (!skMeshData.mesh.vertices.empty())
        {
            auto mesh = meshLifetime->CreateSkeletalMesh(skMeshData.mesh);
            if (!mesh)
            {
                throw std::exception("Cannot create skeletal mesh");
            }

            meshesCache.emplace(skMeshData.mesh.id, mesh);
        }
        else
        {
            bBreak = true;
        }

        if (!preloadDelegatesCache.empty() ||
            !instancesDelegatesCache.empty() ||
            !skeletalDelegatesCache.empty())
        {
            CheckLoadFinished(meshData, skMeshData);
        }

        if (bBreak) break;
    }

    S_CATCH{ S_THROW("SMeshManagerWindows::Update()"); }
}

void SMeshManagerWindows::LoadStaticMeshInstances(const std::filesystem::path& path,
    SGroupID groupId, OnMeshInstancesLoadedDelegate delegate)
{
    SMeshID meshId = ResourceID<SMeshID>(path.string());
    instancesDelegatesCache.emplace_back(meshId, delegate);

    auto LoadInstancesTask = [this, path, meshId, groupId]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, groupId, meshesData))
        {
            meshesData.id = meshId;

            // write in thread pool space
            TLockGuard guard(sync);
            loadedMeshes.push(meshesData);
        }
        else
        {
            DebugMsg("[%s] SMeshManagerWindows::LoadStaticMeshInstances(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadInstancesTask, "Load mesh instances");
}

void SMeshManagerWindows::PreloadStaticMeshes(const std::filesystem::path& path,
    OnMeshesLoadedDelegate delegate)
{
    SMeshID meshId = ResourceID<SMeshID>(path.string());
    preloadDelegatesCache.emplace_back(meshId, delegate);

    auto PreloadMeshesTask = [this, path, meshId]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, 0u, meshesData))
        {
            meshesData.id = meshId;

            // write in thread pool space
            TLockGuard guard(sync);
            loadedMeshes.push(meshesData);
        }
        else
        {
            DebugMsg("[%s] SMeshManagerWindows::PreloadStaticMeshes(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadMeshesTask, "Preload meshes");
}

void SMeshManagerWindows::LoadSkeletalMesh(const std::filesystem::path& path,
    OnSkeletalMeshLoadedDelegate delegate)
{
    SMeshID meshId = ResourceID<SMeshID>(path.string());
    skeletalDelegatesCache.emplace_back(meshId, delegate);

    auto LoadSkeletalMeshTask = [this, path, meshId]()
    {
        SSkeletalMeshData meshData{};
        if (LoadSkeletalMeshData(path, meshData))
        {
            meshData.id = meshId;

            // write in thread pool space
            TLockGuard guard(sync);
            loadedSkeletalMeshes.push(meshData);
        }
        else
        {
            DebugMsg("[%s] SMeshManagerWindows::LoadSkeletalMesh(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadSkeletalMeshTask, "Load skeletal mesh");
}

bool SMeshManagerWindows::LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData)
{
    try
    {
        auto keysView = meshesCache | std::views::keys;
        std::forward_list<SMeshID> ids(keysView.begin(), keysView.end());

        LoadFbxStaticMeshes(path, groupId, ids, meshesData.meshes, meshesData.instances);
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

bool SMeshManagerWindows::LoadSkeletalMeshData(const std::filesystem::path& path, SSkeletalMeshData& meshData)
{
    try
    {
        LoadFbxSkeletalMesh(path, meshData.mesh);
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadSkeletalMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

void SMeshManagerWindows::CheckLoadFinished(const SMeshData& meshData, const SSkeletalMeshData& skMeshData)
{
    // static meshes
    if (!meshData.meshes.empty() || !meshData.instances.empty())
    {
        std::forward_list<TInstancesDelegatesCache::const_iterator> eraseList1;
        std::forward_list<TPreloadDelegatesCache::const_iterator> eraseList2;

        for (auto it = instancesDelegatesCache.begin(); it != instancesDelegatesCache.end(); ++it)
        {
            if (it->first == meshData.id)
            {
                eraseList1.push_front(it);

                if (it->second)
                {
                    // call delegate
                    it->second(it->first, meshData.instances);
                }

                if (!meshData.meshes.empty())
                {
                    DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): static meshes from %u created and added to cache\n",
                        GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.id);
                }

                if (!meshData.instances.empty())
                {
                    DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): mesh instances from %u loaded\n",
                        GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.id);
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
                    it->second(it->first);
                }

                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): static meshes from %u created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.id);
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
                    it->second(skMeshData.mesh.id);
                }

                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): skeletal mesh '%s', id=%u created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), skMeshData.mesh.name.c_str(), skMeshData.mesh.id);
            }
        }

        for (auto eraseIt : eraseList)
        {
            skeletalDelegatesCache.erase(eraseIt);
        }
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
