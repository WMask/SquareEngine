/***************************************************************************
* STextureManagerWindows.h
*/

#ifdef WIN32

#include "RenderSystem/Windows/STextureManagerWindows.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <vector>
#include <ranges>
#include <cmath>
#include <DDSTextureLoader.h>

#pragma comment(lib, "DirectXTK.lib")


namespace SConst
{
    static const std::uint32_t MaxTexturesPerFrame = 8u;
}


STextureManagerWindows::~STextureManagerWindows()
{
    Shutdown();
}

void STextureManagerWindows::Init(IThreadPool* inThreadPool, ITextureLifetime* inTextureLifetime)
{
    threadPool = inThreadPool;
    textureLifetime = inTextureLifetime;
    texturesCache.reserve(128);
}

void STextureManagerWindows::Shutdown()
{
    if (!textureLifetime) return;

    ClearCache(nullptr);
    threadPool = nullptr;
    textureLifetime = nullptr;

    {
        TLockGuard guard(sync);
        TTextureQueue textures;
        TCubemapQueue cubemaps;
        loadedTextures.swap(textures);
        loadedCubemaps.swap(cubemaps);
    }
}

void STextureManagerWindows::Update()
{
    if (!textureLifetime) return;

    S_TRY

    for (std::uint32_t i = 0; i < SConst::MaxTexturesPerFrame; i++)
    {
        SCubemapData cubemapData{};

        if (!loadedTextures.empty())
        {
            STextureData textureData;
            {
                TLockGuard guard(sync);
                if (!loadedTextures.empty())
                {
                    // read in game thread space
                    textureData = loadedTextures.front();
                    loadedTextures.pop();
                }
            }

            if (!textureData.data.empty())
            {
                auto texture = textureLifetime->CreateTexture(textureData);
                if (!texture)
                {
                    throw std::exception("Cannot create texture");
                }

                texturesCache.emplace(textureData.id, texture);

                DebugMsg("[%s] STextureManagerWindows::Update(): texture id=%u created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), textureData.id);
            }
        }
        else if (!loadedCubemaps.empty())
        {
            SCubemapData cubemapData;
            {
                TLockGuard guard(sync);
                if (!loadedCubemaps.empty())
                {
                    // read in game thread space
                    cubemapData = loadedCubemaps.front();
                    loadedCubemaps.pop();
                }
            }

            if (!cubemapData.data.empty())
            {
                auto cubemap = textureLifetime->CreateCubemap(cubemapData);
                if (!cubemap)
                {
                    throw std::exception("Cannot create cubemap");
                }

                RemoveCubemap(cubemapData.type);
                cubemapsCache.emplace(cubemapData.type, cubemap);

                DebugMsg("[%s] STextureManagerWindows::Update(): cubemap type=%s created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(),
                    SConst::GetNameByType(cubemapData.type).data());
            }
        }
        else
        {
            break;
        }
    }

    if (!preloadDelegatesCache.empty())
    {
        CheckPreloadFinished();
    }

    S_CATCH{ S_THROW("STextureManagerWindows::Update()"); }
}

STexID STextureManagerWindows::LoadTexture(const std::filesystem::path& path)
{
    STexID id = ResourceID<STexID>(path.string());
    DebugMsg("[%s] STextureManagerWindows::LoadTexture(): begin loading '%s', id=%u\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);

    auto LoadTextureTask = [this, path, id]()
    {
        STextureData texture{};
        if (LoadTextureData(path, &texture.data, &texture.texSize))
        {
            texture.id = id;

            // write in thread pool space
            TLockGuard guard(sync);
            loadedTextures.push(texture);
        }
        else
        {
            DebugMsg("[%s] STextureManagerWindows::LoadTexture(): cannot load '%s', id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadTextureTask, "Load texture");
    return id;
}

void STextureManagerWindows::PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate)
{
    // transform paths to ids
    auto viewIds = paths | std::views::transform([](const auto& path) {
        return ResourceID<STexID>(path.string());
    });
    TTexIDList ids(viewIds.begin(), viewIds.end());

    // cache delegate and ids
    preloadDelegatesCache.emplace_back(ids, delegate);

    auto PreloadTexturesTask = [this, paths, ids]()
    {
        std::vector<STextureData> textures;
        textures.reserve(paths.size());

        for (auto i = 0; i < paths.size(); i++)
        {
            auto& path = paths[i];
            auto id = ids[i];

            DebugMsg("[%s] STextureManagerWindows::PreloadTextures(): begin loading '%s', id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);

            STextureData texture{};
            if (LoadTextureData(path, &texture.data, &texture.texSize))
            {
                texture.id = id;
                textures.push_back(texture);
            }
            else
            {
                DebugMsg("[%s] STextureManagerWindows::PreloadTextures(): cannot load '%s'\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
            }
        }

        // write in thread pool space
        TLockGuard guard(sync);
        for (auto& texture : textures)
        {
            loadedTextures.push(texture);
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadTexturesTask, "Preload textures");
}

STextureBase* STextureManagerWindows::FindTexture(STexID id) const
{
    const auto it = texturesCache.find(id);
    if (it != texturesCache.end())
    {
        return it->second.get();
    }

    return nullptr;
}

bool STextureManagerWindows::RemoveTexture(STexID id)
{
    if (texturesCache.contains(id))
    {
        texturesCache.erase(id);
        return true;
    }

    return false;
}

void STextureManagerWindows::LoadCubemap(const std::filesystem::path& path, ECubemapType type)
{
    DebugMsg("[%s] STextureManagerWindows::LoadCubemap(): begin loading '%s', type=%s\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(),
        SConst::GetNameByType(type).data());

    auto LoadCubemapTask = [this, path, type]()
    {
        bool bLoadFailed = false;
        try
        {
            SCubemapData cubemap;
            cubemap.data = ReadBinaryFile(path);
            if (!cubemap.data.empty())
            {
                cubemap.type = type;

                // write in thread pool space
                TLockGuard guard(sync);
                loadedCubemaps.push(cubemap);
            }
            else
            {
                bLoadFailed = true;
            }
        }
        catch (const std::exception&)
        {
            bLoadFailed = true;
        }

        if (bLoadFailed)
        {
            DebugMsg("[%s] STextureManagerWindows::LoadCubemap(): cannot load '%s', type=%s\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(),
                SConst::GetNameByType(type).data());
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadCubemapTask, "Load cubemap");
}

STextureBase* STextureManagerWindows::FindCubemap(ECubemapType type) const
{
    auto it = cubemapsCache.find(type);
    if (it != cubemapsCache.end())
    {
        return it->second.get();
    }

    return nullptr;
}

bool STextureManagerWindows::RemoveCubemap(ECubemapType type)
{
    if (cubemapsCache.contains(type))
    {
        cubemapsCache.erase(type);
        return true;
    }

    return false;
}

bool STextureManagerWindows::LoadTextureData(const std::filesystem::path& path, SBytes* outData, SSize2* outTexSize)
{
    try
    {
        SBytes data;
        std::uint32_t width, height, bpp, rowBytes;
        ReadPngFile(path, &width, &height, &bpp, &rowBytes);
        data.resize(rowBytes * height);
        ReadPngFile(path, 0, 0, 0, 0, data.data());

        if (outData) *outData = std::move(data);
        if (outTexSize) *outTexSize = SSize2{ width, height };
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] STextureManagerWindows::LoadTextureData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

void STextureManagerWindows::CheckPreloadFinished()
{
    std::forward_list<TPreLoadDelegatesCache::const_iterator> eraseList;

    for (auto it = preloadDelegatesCache.begin(); it != preloadDelegatesCache.end(); ++it)
    {
        bool bAllLoaded = true;

        for (auto id : it->first)
        {
            if (!texturesCache.contains(id))
            {
                bAllLoaded = false;
                break;
            }
        }

        if (bAllLoaded)
        {
            eraseList.push_front(it);

            if (it->second)
            {
                it->second(it->first);
            }
        }
    }

    for (auto eraseIt : eraseList)
    {
        preloadDelegatesCache.erase(eraseIt);
    }
}

void STextureManagerWindows::ClearCache(IWorld* world)
{
    S_TRY

    if (world)
    {
        std::set<STexID> aliveTexList;
        const auto& registry = world->GetEntities();
        const auto& view = registry.view<STexturedComponent>();

        view.each([&aliveTexList](const STexturedComponent& textureComponent) {
            aliveTexList.insert(textureComponent.id);
        });

        std::set<STexID> eraseTexList;
        for (auto tex : texturesCache)
        {
            const bool bNotFound = (aliveTexList.find(tex.first) == aliveTexList.end());
            if (bNotFound)
            {
                eraseTexList.insert(tex.first);
            }
        }

        for (auto entry : eraseTexList)
        {
            texturesCache.erase(entry);
        }
    }
    else
    {
        texturesCache.clear();
    }

    cubemapsCache.clear();

    S_CATCH{ S_THROW("STextureManagerWindows::ClearCache()") }
}

#endif // WIN32
