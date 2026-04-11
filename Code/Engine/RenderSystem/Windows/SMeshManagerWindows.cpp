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
    static const std::uint32_t MaxMeshes = 64u;
    static const std::uint32_t MaxMeshesPerFrame = 8u;
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
    ClearCache(nullptr);
    threadPool = nullptr;

    {
        TLockGuard guard(sync);
        TMeshQueue meshes;
        loadedMeshes.swap(meshes);
        meshesCacheIds.clear();
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
        {
            TLockGuard guard(sync);
            if (!loadedMeshes.empty())
            {
                // read in game thread space
                meshData = loadedMeshes.front();
                loadedMeshes.pop();
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

            UpdateCacheIds();
        }
        else
        {
            bBreak = true;
        }

        if (!preloadDelegatesCache.empty() || !instancesDelegatesCache.empty())
        {
            CheckLoadFinished(meshData);
        }

        if (bBreak) break;
    }

    S_CATCH{ S_THROW("SMeshManagerWindows::Update()"); }
}

void SMeshManagerWindows::LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate)
{
    instancesDelegatesCache.emplace_back(path, delegate);

    auto LoadInstancesTask = [this, path, groupId]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, groupId, meshesData))
        {
            meshesData.id = ResourceID<SMeshID>(path.string());

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

void SMeshManagerWindows::PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate)
{
    preloadDelegatesCache.emplace_back(path, delegate);

    auto PreloadMeshesTask = [this, path]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, 0u, meshesData))
        {
            meshesData.id = ResourceID<SMeshID>(path.string());

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

bool SMeshManagerWindows::LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData)
{
    try
    {
        // read in thread pool space
        TMeshesCacheIds cachedIdsCopy;
        {
            TLockGuard guard(sync);
            cachedIdsCopy = meshesCacheIds;
        }

        LoadFbxStaticMeshes(path, groupId, cachedIdsCopy, meshesData.meshes, meshesData.instances);
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerWindows::LoadMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}
/*
bool SMeshManagerWindows::CreateMesh(ID3D11Device* device, const SMesh& meshData, SMeshDataDX11& outMesh)
{
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = sizeof(SVertex) * meshData.vertices.size();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA bufferData{};
    bufferData.pSysMem = meshData.vertices.data();

    // create vertex buffer
    if (FAILED(device->CreateBuffer(&bufferDesc, &bufferData, outMesh.vb.GetAddressOf())))
    {
        DebugMsg("[%s] SMeshManagerWindows::CreateMesh(): Cannot create vertex buffer\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str());
        return false;
    }

    // create index buffer
    bufferDesc.ByteWidth = sizeof(std::uint16_t) * meshData.indices.size();
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferData.pSysMem = meshData.indices.data();

    if (FAILED(device->CreateBuffer(&bufferDesc, &bufferData, outMesh.ib.GetAddressOf())))
    {
        DebugMsg("[%s] SMeshManagerWindows::CreateMesh(): Cannot create index buffer\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str());
        return false;
    }

    outMesh.materials = meshData.materials;

    // load textures
    if (renderSystem)
    {
        for (auto& material : meshData.materials)
        {
            SPathList paths;
            auto [baseView, baseSize] = renderSystem->FindTexture(ResourceID<STexID>(material.baseTexture.string()));
            if (!baseView && !material.baseTexture.empty()) paths.push_back(material.baseTexture);
            auto [normView, normSize] = renderSystem->FindTexture(ResourceID<STexID>(material.normTexture.string()));
            if (!normView && !material.normTexture.empty()) paths.push_back(material.normTexture);
            auto [ormView, ormSize] = renderSystem->FindTexture(ResourceID<STexID>(material.rmaTexture.string()));
            if (!ormView && !material.rmaTexture.empty()) paths.push_back(material.rmaTexture);
            auto [emiView, emiSize] = renderSystem->FindTexture(ResourceID<STexID>(material.emiTexture.string()));
            if (!ormView && !material.emiTexture.empty()) paths.push_back(material.emiTexture);

            if (!paths.empty())
            {
                static auto onLoaded = [](std::vector<std::filesystem::path>& textures)
                {
                    for (auto& texture : textures)
                    {
                        DebugMsg("[%s] SMeshManagerWindows::CreateMesh(): material texture loaded '%s'\n",
                            GetTimeStamp(std::chrono::system_clock::now()).c_str(), texture.string().c_str());
                    }
                };
                renderSystem->GetTextureManager().PreloadTextures(paths, onLoaded);
            }
        }
    }

    DebugMsg("[%s] SMeshManagerWindows::CreateMesh(): mesh '%s' created and added to cache\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.name.c_str());

    return true;
}
*/
void SMeshManagerWindows::CheckLoadFinished(const SMeshData& meshData)
{
    std::forward_list<TInstancesDelegatesCache::const_iterator> eraseList1;
    std::forward_list<TPreloadDelegatesCache::const_iterator> eraseList2;

    for (auto it = instancesDelegatesCache.begin(); it != instancesDelegatesCache.end(); ++it)
    {
        auto path = it->first.string();
        auto id = ResourceID<SMeshID>(path);

        if (id == meshData.id)
        {
            eraseList1.push_front(it);

            if (it->second)
            {
                // call delegate
                it->second(it->first, meshData.instances);
            }

            if (!meshData.meshes.empty())
            {
                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): meshes from '%s' created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
            }

            if (!meshData.instances.empty())
            {
                DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): instances from '%s' loaded\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
            }
        }
    }

    for (auto it = preloadDelegatesCache.begin(); it != preloadDelegatesCache.end(); ++it)
    {
        auto path = it->first.string();
        auto id = ResourceID<SMeshID>(path);

        if (id == meshData.id)
        {
            eraseList2.push_front(it);

            if (it->second)
            {
                // call delegate
                it->second(it->first);
            }

            DebugMsg("[%s] SMeshManagerWindows::CheckLoadFinished(): meshes from '%s' created and added to cache\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
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

void SMeshManagerWindows::ClearCache(IWorld* world)
{
    S_TRY

    if (world)
    {
        std::set<STexID> aliveMeshList;
        const auto& registry = world->GetEntities();
        const auto& view = registry.view<SStaticMeshComponent>();

        view.each([&aliveMeshList](const SStaticMeshComponent& meshComponent) {
            aliveMeshList.insert(meshComponent.id);
        });

        std::set<STexID> eraseTexList;
        for (auto tex : meshesCache)
        {
            const bool bNotFound = (aliveMeshList.find(tex.first) == aliveMeshList.end());
            if (bNotFound)
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

    UpdateCacheIds();

    S_CATCH{ S_THROW("SMeshManagerWindows::ClearCache()") }
}

void SMeshManagerWindows::UpdateCacheIds()
{
    auto keysView = meshesCache | std::views::keys;
    auto updatedIds = std::forward_list<SMeshID>(keysView.begin(), keysView.end());

    // write in game thread space
    TLockGuard guard(sync);
    meshesCacheIds = updatedIds;
}

#endif // WIN32
