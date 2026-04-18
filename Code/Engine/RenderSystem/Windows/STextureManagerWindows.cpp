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
    constexpr std::uint32_t MaxTexturesPerFrame = 4u;
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
        STextureData textureData{};
        SCubemapData cubemapData{};
        if (!loadedTextures.empty() ||
            !loadedCubemaps.empty())
        {
            TLockGuard guard(sync);
            // read in game thread space
            if (!loadedTextures.empty())
            {
                textureData = loadedTextures.front();
                loadedTextures.pop();
            }

            if (!loadedCubemaps.empty())
            {
                cubemapData = loadedCubemaps.front();
                loadedCubemaps.pop();
            }
        }

        if (!textureData.data.empty() && !textureData.bLoadFailed)
        {
            auto texture = textureLifetime->CreateTexture(textureData);
            if (!texture)
            {
                throw std::exception("Cannot create texture");
            }

            texturesCache.emplace(textureData.id, texture);

            DebugMsg("[%s] STextureManagerWindows::Update(): texture '%s', id=%u created and added to cache\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(),
                textureData.path.string().c_str(), textureData.id);
        }

        if (!cubemapData.data.empty() && !cubemapData.bLoadFailed)
        {
            auto cubemap = textureLifetime->CreateCubemap(cubemapData);
            if (!cubemap)
            {
                throw std::exception("Cannot create cubemap");
            }

            RemoveCubemap(cubemapData.type);
            cubemapsCache.emplace(cubemapData.type, cubemap);

            DebugMsg("[%s] STextureManagerWindows::Update(): cubemap '%s', type=%s created and added to cache\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), cubemapData.path.string().c_str(),
                SConst::GetNameByType(cubemapData.type).data());
        }

        if (!preloadDelegatesCache.empty())
        {
            CheckPreloadFinished(textureData);
        }

        if (textureData.data.empty() &&
            cubemapData.data.empty())
        {
            break;
        }
    }

    S_CATCH{ S_THROW("STextureManagerWindows::Update()"); }
}

STexID STextureManagerWindows::LoadTexture(const SPath& path)
{
    STexID id = ResourceID<STexID>(path.string());
    DebugMsg("[%s] STextureManagerWindows::LoadTexture(): begin loading '%s', id=%u\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);

    auto LoadTextureTask = [this, path, id]()
    {
        STextureData texture{};
        texture.id = id;
        texture.path = path;
        if (!LoadTextureData(path, &texture.data, &texture.texSize))
        {
            texture.bLoadFailed = true;
            texture.data.clear();
            DebugMsg("[%s] STextureManagerWindows::LoadTexture(): cannot load '%s', id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);
        }

        // write in thread pool space
        TLockGuard guard(sync);
        loadedTextures.push(std::move(texture));
    };

    // send task to thread pool
    threadPool->AddTask(LoadTextureTask, "Load texture");
    return id;
}

void STextureManagerWindows::PreloadTextures(const SPathList& paths)
{
    static OnTexturesLoadedDelegate delegate;
    PreloadTextures(paths, delegate);
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
            // save id even with failed data to call delegate
            texture.id = id;
            texture.path = path;
            if (!LoadTextureData(path, &texture.data, &texture.texSize))
            {
                texture.bLoadFailed = true;
                texture.data.clear();
                DebugMsg("[%s] STextureManagerWindows::PreloadTextures(): cannot load '%s'\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str());
            }

            textures.push_back(texture);
        }

        // write in thread pool space
        TLockGuard guard(sync);
        for (auto& texture : textures)
        {
            loadedTextures.push(std::move(texture));
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadTexturesTask, "Preload textures");
}

std::pair<SSize2, bool> STextureManagerWindows::GetTextureSize(STexID id) const
{
    std::pair<SSize2, bool> zero{ SConst::ZeroSSize2, false };
    return textureLifetime ? textureLifetime->GetTextureSize(id) : zero;
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
    const auto it = texturesCache.find(id);
    if (it != texturesCache.end())
    {
        texturesCache.erase(it);
        return true;
    }

    return false;
}

void STextureManagerWindows::LoadCubemap(const SPath& path, ECubemapType type)
{
    DebugMsg("[%s] STextureManagerWindows::LoadCubemap(): begin loading '%s', type=%s\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(),
        SConst::GetNameByType(type).data());

    auto LoadCubemapTask = [this, path, type]()
    {
        SCubemapData cubemap{};
        cubemap.type = type;
        cubemap.path = path;
        if (!LoadCubemapData(path, &cubemap.data))
        {
            cubemap.bLoadFailed = true;
            cubemap.data.clear();
        }

        // write in thread pool space
        TLockGuard guard(sync);
        loadedCubemaps.push(std::move(cubemap));
    };

    // send task to thread pool
    threadPool->AddTask(LoadCubemapTask, "Load cubemap");
}

STextureBase* STextureManagerWindows::FindCubemap(ECubemapType type) const
{
    const auto it = cubemapsCache.find(type);
    if (it != cubemapsCache.end())
    {
        return it->second.get();
    }

    return nullptr;
}

bool STextureManagerWindows::RemoveCubemap(ECubemapType type)
{
    const auto it = cubemapsCache.find(type);
    if (it != cubemapsCache.end())
    {
        cubemapsCache.erase(it);
        return true;
    }

    return false;
}

bool STextureManagerWindows::LoadTextureData(const SPath& path, SBytes* outData, SSize2* outTexSize)
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

bool STextureManagerWindows::LoadCubemapData(const SPath& path, SBytes* outData)
{
    try
    {
        SBytes data = ReadBinaryFile(path);
        if (outData) *outData = std::move(data);
    }
    catch (const std::exception& ex)
    {
        DebugMsg("[%s] STextureManagerWindows::LoadCubemapData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

void STextureManagerWindows::CheckPreloadFinished(const STextureData& textureData)
{
    std::forward_list<TPreLoadDelegatesCache::const_iterator> eraseList;

    for (auto it = preloadDelegatesCache.begin(); it != preloadDelegatesCache.end(); ++it)
    {
        bool bAllLoaded = true;
        bool bLoadFailed = false;

        for (auto id : it->first)
        {
            if (textureData.id == id &&
                textureData.bLoadFailed)
            {
                bLoadFailed = true;
                break;
            }

            if (!texturesCache.contains(id))
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
                it->second(false, it->first);
            }
        }
        else if (bAllLoaded)
        {
            eraseList.push_front(it);

            if (it->second)
            {
                it->second(true, it->first);
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
