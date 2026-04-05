/***************************************************************************
* STextureManagerDX11.h
*/

#ifdef WIN32

#include "RenderSystem/DX11/STextureManagerDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#define USE_DIRECTX_TK 1
#if USE_DIRECTX_TK
// Install directxtk_desktop_win10 by Nuget package manager
# include <DDSTextureLoader.h>
# pragma comment(lib, "DirectXTK.lib")
#endif

#include <vector>
#include <cmath>


namespace SConst
{
    static const std::uint32_t MaxTextures = 64u;
    static const std::uint32_t MaxTexturesPerFrame = 8u;
}

STextureManagerDX11::~STextureManagerDX11()
{
    Shutdown();
}

void STextureManagerDX11::Init(IThreadPool* inThreadPool)
{
    threadPool = inThreadPool;
    loadedTextures = std::make_shared<TCircularFIFOTextureQueue>(SConst::MaxTextures);
    texturesCache.reserve(128);
}

void STextureManagerDX11::Shutdown()
{
    ClearCache(nullptr);
    loadedTextures.reset();
    threadPool = nullptr;
    RemoveCubemap();
}

bool STextureManagerDX11::FindTexture(STexID id, ID3D11Texture2D** outTexture,
    ID3D11ShaderResourceView** outView, SSize2* outTexSize) const
{
    if (texturesCache.contains(id))
    {
        const auto& it = texturesCache.find(id);
        auto& texData = it->second;
        *outTexture = texData.texture.Get();
        *outView = texData.view.Get();
        if (outTexSize) *outTexSize = texData.texSize;
        return true;
    }

    return false;
}

bool STextureManagerDX11::SetCubemap(const std::filesystem::path& path, ID3D11Device* d3dDevice)
{
#if USE_DIRECTX_TK
    if (FAILED(DirectX::CreateDDSTextureFromFile(d3dDevice, path.c_str(), cubemap.GetAddressOf(), cubemapView.GetAddressOf())))
    {
        throw std::exception("STextureManagerDX11::SetCubemap(): cannot load cubemap texture");
    }
#endif
    return cubemapView;
}

void STextureManagerDX11::RemoveCubemap()
{
    if (cubemapView) cubemapView.Reset();
    if (cubemap) cubemap.Reset();
}

void STextureManagerDX11::Update(ID3D11Device* d3dDevice)
{
    S_TRY

    if (d3dDevice && loadedTextures)
    {
        for (std::uint32_t i = 0; i < SConst::MaxTexturesPerFrame; i++)
        {
            // read in game thread space
            STextureData data{};
            if (loadedTextures->pop(data))
            {
                STextureDataDX11 texture{};
                if (!CreateTexture(d3dDevice, data, texture))
                {
                    throw std::exception("Cannot create texture");
                }

                texturesCache.emplace(data.id, std::move(texture));

                DebugMsg("[%s] STextureManagerDX11::Update(): texture id=%u created and added to cache\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), data.id);

                if (!preLoadDelegatesCache.empty())
                {
                    CheckPreloadFinished();
                }
            }
            else
            {
                break;
            }
        }
    }

    S_CATCH{ S_THROW("STextureManagerDX11::Update()"); }
}

STexID STextureManagerDX11::LoadTexture(const std::filesystem::path& path)
{
    STexID id = ResourceID<STexID>(path.string());
    DebugMsg("[%s] STextureManagerDX11::LoadTexture(): begin loading '%s', id=%u\n",
        GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);

    auto LoadTextureTask = [this, path, id]()
    {
        STextureData texture{};
        if (LoadTextureData(path, &texture.data, &texture.texSize))
        {
            texture.id = id;

            // write in thread pool space
            loadedTextures->push(texture);
        }
        else
        {
            DebugMsg("[%s] STextureManagerDX11::LoadTexture(): cannot load '%s', id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.string().c_str(), id);
        }
    };

    // send task to thread pool
    threadPool->AddTask(LoadTextureTask, "Load texture");
    return id;
}

void STextureManagerDX11::PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate)
{
    preLoadDelegatesCache.emplace_back(paths, delegate);

    auto PreloadTexturesTask = [this, paths]()
    {
        for (auto& entry : paths)
        {
            STexID id = ResourceID<STexID>(entry.string());
            DebugMsg("[%s] STextureManagerDX11::PreloadTextures(): begin loading '%s', id=%u\n",
                GetTimeStamp(std::chrono::system_clock::now()).c_str(), entry.string().c_str(), id);

            STextureData texture{};
            if (LoadTextureData(entry, &texture.data, &texture.texSize))
            {
                texture.id = id;

                // write in thread pool space
                loadedTextures->push(texture);
            }
            else
            {
                DebugMsg("[%s] STextureManagerDX11::PreloadTextures(): cannot load '%s'\n",
                    GetTimeStamp(std::chrono::system_clock::now()).c_str(), entry.string().c_str());
            }
        }
    };

    // send task to thread pool
    threadPool->AddTask(PreloadTexturesTask, "Preload textures");
}

bool STextureManagerDX11::LoadTextureData(const std::filesystem::path& path, SBytes* outData, SSize2* outTexSize)
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
        DebugMsg("[%s] STextureManagerDX11::LoadTextureData(): %s\n",
            GetTimeStamp(std::chrono::system_clock::now()).c_str(), ex.what());
        return false;
    }

    return true;
}

bool STextureManagerDX11::CreateTexture(ID3D11Device* d3dDevice, const STextureData& textureData, STextureDataDX11& outTexture)
{
    D3D11_SUBRESOURCE_DATA initData = {
        textureData.data.data(),
        static_cast<UINT>(textureData.texSize.width * 4),
        static_cast<UINT>(textureData.data.size())
    };

    // create texture
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = textureData.texSize.width;
    desc.Height = textureData.texSize.height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ComPtr<ID3D11Texture2D> newTexture;
    if (SUCCEEDED(d3dDevice->CreateTexture2D(&desc, &initData, newTexture.GetAddressOf())))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
        SRVDesc.Format = desc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        ComPtr<ID3D11ShaderResourceView> newView;
        if (SUCCEEDED(d3dDevice->CreateShaderResourceView(newTexture.Get(), &SRVDesc, newView.GetAddressOf())))
        {
            outTexture.texture = std::move(newTexture);
            outTexture.view = std::move(newView);
            outTexture.texSize = textureData.texSize;
            return true;
        }
    }

    return false;
}

void STextureManagerDX11::CheckPreloadFinished()
{
    std::forward_list<TPreLoadDelegatesCache::const_iterator> eraseList;

    for (auto it = preLoadDelegatesCache.begin(); it != preLoadDelegatesCache.end(); ++it)
    {
        bool bAllLoaded = true;

        for (auto path : it->first)
        {
            auto id = ResourceID<STexID>(path.string());
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
        preLoadDelegatesCache.erase(eraseIt);
    }
}

void STextureManagerDX11::ClearCache(IWorld* world)
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

    S_CATCH{ S_THROW("STextureManagerDX11::ClearCache()") }
}

#endif // WIN32
