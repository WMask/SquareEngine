/***************************************************************************
* SMeshManagerDX11.h
*/

#ifdef WIN32

#include "RenderSystem/DX11/SMeshManagerDX11.h"
#include "RenderSystem/DX11/SRenderSystemDX11.h"
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

SMeshManagerDX11::~SMeshManagerDX11()
{
    Shutdown();
}

void SMeshManagerDX11::Init(IThreadPool* inThreadPool, SRenderSystemDX11* inRenderSystem)
{
    threadPool = inThreadPool;
    renderSystem = inRenderSystem;
    loadedMeshes = std::make_shared<TCircularFIFOTextureQueue>(SConst::MaxMeshes);
    auto initialIds = std::make_shared<std::forward_list<SMeshID>>();
    meshesCacheIds.store(initialIds, std::memory_order_release);
    meshesCache.reserve(128);
}

void SMeshManagerDX11::Shutdown()
{
    ClearCache(nullptr);
    auto empty = std::make_shared<std::forward_list<SMeshID>>();
    meshesCacheIds.store(empty);
    loadedMeshes.reset();
    threadPool = nullptr;
}

bool SMeshManagerDX11::FindMesh(SMeshID id, std::vector<SMaterial>* outMaterials, ID3D11Buffer** outVB, ID3D11Buffer** outIB) const
{
    if (meshesCache.contains(id))
    {
        const auto& it = meshesCache.find(id);
        auto& texData = it->second;
        *outVB = texData.vb.Get();
        *outIB = texData.ib.Get();
        *outMaterials = texData.materials;
        return true;
    }

    return false;
}

void SMeshManagerDX11::Update(ID3D11Device* d3dDevice)
{
    S_TRY

    if (d3dDevice && loadedMeshes)
    {
        for (std::uint32_t i = 0; i < SConst::MaxMeshesPerFrame; i++)
        {
            // read in game thread space
            SMeshData meshData{};
            if (loadedMeshes->pop(meshData))
            {
                for (auto& data : meshData.meshes)
                {
                    SMeshDataDX11 mesh{};
                    if (!CreateMesh(d3dDevice, data, mesh))
                    {
                        throw std::exception("Cannot create mesh");
                    }

                    meshesCache.emplace(data.id, std::move(mesh));
                }

                UpdateCacheIds();

                if (!preloadDelegatesCache.empty() || !instancesDelegatesCache.empty())
                {
                    CheckLoadFinished(meshData);
                }
            }
            else
            {
                break;
            }
        }
    }

    S_CATCH{ S_THROW("SMeshManagerDX11::Update()"); }
}

void SMeshManagerDX11::LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate)
{
    instancesDelegatesCache.emplace_back(path, delegate);

    auto LoadInstancesTask = [this, path, groupId]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, groupId, meshesData))
        {
            meshesData.id = ResourceID<SMeshID>(path.string());

            // write in thread pool space
            loadedMeshes->push(meshesData);
        }
        else
        {
            DebugMsg("[%s] SMeshManagerDX11::LoadStaticMeshInstances(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadInstancesTask, "Load mesh instances");
}

void SMeshManagerDX11::PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate)
{
    preloadDelegatesCache.emplace_back(path, delegate);

    auto PreloadMeshesTask = [this, path]()
    {
        SMeshData meshesData{};
        if (LoadMeshData(path, 0u, meshesData))
        {
            meshesData.id = ResourceID<SMeshID>(path.string());

            // write in thread pool space
            loadedMeshes->push(meshesData);
        }
        else
        {
            DebugMsg("[%s] SMeshManagerDX11::PreloadStaticMeshes(): cannot load '%s'\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadMeshesTask, "Preload meshes");
}

bool SMeshManagerDX11::LoadMeshData(const std::filesystem::path& path, SGroupID groupId, SMeshData& meshesData)
{
    try
    {
        // read in thread pool space
        auto cachedIdsCopy = meshesCacheIds.load(std::memory_order_acquire);

        LoadFbxStaticMeshes(path, groupId, *cachedIdsCopy.get(), meshesData.meshes, meshesData.instances);
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] SMeshManagerDX11::LoadMeshData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

bool SMeshManagerDX11::CreateMesh(ID3D11Device* device, const SMesh& meshData, SMeshDataDX11& outMesh)
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
        DebugMsg("[%s] SMeshManagerDX11::CreateMesh(): Cannot create vertex buffer\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str());
        return false;
    }

    // create index buffer
    bufferDesc.ByteWidth = sizeof(std::uint16_t) * meshData.indices.size();
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferData.pSysMem = meshData.indices.data();

    if (FAILED(device->CreateBuffer(&bufferDesc, &bufferData, outMesh.ib.GetAddressOf())))
    {
        DebugMsg("[%s] SMeshManagerDX11::CreateMesh(): Cannot create index buffer\n",
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
            auto [ormView, ormSize] = renderSystem->FindTexture(ResourceID<STexID>(material.ormTexture.string()));
            if (!ormView && !material.ormTexture.empty()) paths.push_back(material.ormTexture);

            if (!paths.empty())
            {
                static auto onLoaded = [](std::vector<std::filesystem::path>& textures)
                {
                    for (auto& texture : textures)
                    {
                        DebugMsg("[%s] SMeshManagerDX11::CreateMesh(): material texture loaded '%s'\n",
                            GetTimeStamp(std::chrono::system_clock::now()).c_str(), texture.string().c_str());
                    }
                };
                renderSystem->GetTextureManager().PreloadTextures(paths, onLoaded);
            }
        }
    }

    DebugMsg("[%s] SMeshManagerDX11::CreateMesh(): mesh '%s' created and added to cache\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshData.name.c_str());

    return true;
}

void SMeshManagerDX11::CheckLoadFinished(const SMeshData& meshData)
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
                DebugMsg("[%s] SMeshManagerDX11::CheckLoadFinished(): meshes from '%s' created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
            }

            if (!meshData.instances.empty())
            {
                DebugMsg("[%s] SMeshManagerDX11::CheckLoadFinished(): instances from '%s' loaded\n",
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

            DebugMsg("[%s] SMeshManagerDX11::CheckLoadFinished(): meshes from '%s' created and added to cache\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
        }
    }

    for (auto eraseIt : eraseList2)
    {
        preloadDelegatesCache.erase(eraseIt);
    }
}

void SMeshManagerDX11::ClearCache(IWorld* world)
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

    S_CATCH{ S_THROW("SMeshManagerDX11::ClearCache()") }
}

void SMeshManagerDX11::UpdateCacheIds()
{
    auto keysView = meshesCache | std::views::keys;
    auto updatedIds = std::make_shared<std::forward_list<SMeshID>>(keysView.begin(), keysView.end());

    // write in game thread space
    meshesCacheIds.store(updatedIds, std::memory_order_release);
}

#endif // WIN32
