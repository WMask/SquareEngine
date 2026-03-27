/***************************************************************************
* SWindowsUtils.h
*/

#ifdef WIN32

#include "RenderSystem/Windows/SWindowsUtils.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <wrl.h>
#include <vector>
#include <cmath>

#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;


std::vector<ComPtr<IDXGIAdapter>> SEnumerateAdapters()
{
    std::vector<ComPtr<IDXGIAdapter>> adapters;
    ComPtr<IDXGIFactory> factory;

    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()))))
    {
        return adapters;
    }

    IDXGIAdapter* adapter;
    for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        adapters.push_back(adapter);
    }

    return adapters;
}

bool SFindDisplayMode(std::int32_t width, std::int32_t height, std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode)
{
    if (!outMode) return false;

    outMode->RefreshRate.Numerator = 1;
    outMode->RefreshRate.Denominator = 1;
    outMode->Format = DXGI_FORMAT_UNKNOWN;

    auto adapters = SEnumerateAdapters();
    for (auto adapter : adapters)
    {
        ComPtr<IDXGIOutput> output;
        if (SUCCEEDED(adapter->EnumOutputs(0, output.GetAddressOf())))
        {
            UINT numModes = 0;
            std::vector<DXGI_MODE_DESC> displayModes;
            DXGI_FORMAT format = SConst::DefaultBackBufferFormat;

            output->GetDisplayModeList(format, 0, &numModes, NULL);
            displayModes.resize(numModes);

            output->GetDisplayModeList(format, 0, &numModes, &displayModes[0]);
            for (auto& mode : displayModes)
            {
                const UINT RefreshRate = mode.RefreshRate.Numerator / mode.RefreshRate.Denominator;
                if (mode.Width == width &&
                    mode.Height == height &&
                    RefreshRate >= 56 &&
                    RefreshRate <= maxRefreshRate &&
                    mode.Format == SConst::DefaultBackBufferFormat)
                {
                    float prevRate = static_cast<float>(outMode->RefreshRate.Numerator) / static_cast<float>(outMode->RefreshRate.Denominator);
                    float curRate = static_cast<float>(mode.RefreshRate.Numerator) / static_cast<float>(mode.RefreshRate.Denominator);
                    if (curRate > prevRate)
                    {
                        *outMode = mode;
                    }
                }
            }
        }
    }

    return (outMode->Format != DXGI_FORMAT_UNKNOWN);
}

void SMakeWindowAssociation(HWND hWnd)
{
    ComPtr<IDXGIFactory> factory;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()))))
    {
        throw std::exception("SMakeWindowAssociation(): Cannot create factory");
    }

    if (FAILED(factory->MakeWindowAssociation(hWnd, 0)))
    {
        throw std::exception("SMakeWindowAssociation(): Cannot make window association");
    }
}

STextureManagerDX11::STextureManagerDX11()
{
    texturesCache.reserve(128);
    texturesCache.reserve(128);
}

STextureManagerDX11::~STextureManagerDX11()
{
    Shutdown();
}

void STextureManagerDX11::Shutdown()
{
    ClearCache(nullptr);
}

bool STextureManagerDX11::FindTexture(STexID id, ID3D11Texture2D** outTexture,
    ID3D11ShaderResourceView** outView, SSize2* outTexSize)
{
    if (texturesCache.contains(id))
    {
        auto& texData = texturesCache[id];
        *outTexture = texData.texture.Get();
        *outView = texData.view.Get();
        if (outTexSize) *outTexSize = texData.texSize;
        return true;
    }

    return false;
}

std::pair<STexID, bool> STextureManagerDX11::LoadTexture(const std::filesystem::path& texPath,
    ID3D11Device* device, ID3D11Texture2D** outTexture, ID3D11ShaderResourceView** outView,
    SSize2* outTexSize)
{
    S_TRY

    if (!device)
    {
        throw std::exception("Invalid function arguments");
    }

    // make texture name and id
    std::string name = texPath.string();
    size_t dotsPos = name.rfind("..");
    if (dotsPos != std::string::npos)
    {
        name = name.substr(dotsPos + 2);
    }
    STexID nameId = hasher(name);

    // get hashed texture
    if (texturesCache.contains(nameId))
    {
        auto& texData = texturesCache[nameId];
        if (outTexture) *outTexture = texData.texture.Get();
        if (outView) *outView = texData.view.Get();
        if (outTexSize) *outTexSize = texData.texSize;
        return { nameId, true };
    }

    // read new texture from file
    std::vector<std::uint8_t> data;
    std::uint32_t width, height, bpp, rowBytes;
    ReadPngFile(texPath, &width, &height, &bpp, &rowBytes);
    data.resize(rowBytes * height);
    ReadPngFile(texPath, 0, 0, 0, 0, data.data());
    const BYTE* texPixelsPtr = data.data();

    // create texture
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = { texPixelsPtr, static_cast<UINT>(width * 4), static_cast<UINT>(data.size()) };

    ComPtr<ID3D11Texture2D> newTexture;
    if (SUCCEEDED(device->CreateTexture2D(&desc, &initData, newTexture.GetAddressOf())))
    {
        STextureDataDX11 newTexData;
        newTexData.texture = std::move(newTexture);
        newTexData.texSize = SSize2{ static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
        SRVDesc.Format = desc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        ComPtr<ID3D11ShaderResourceView> newView;
        if (SUCCEEDED(device->CreateShaderResourceView(newTexData.texture.Get(), &SRVDesc, newView.GetAddressOf())))
        {
            newTexData.view = std::move(newView);
        }

        if (outTexture) *outTexture = newTexData.texture.Get();
        if (outTexSize) *outTexSize = newTexData.texSize;
        if (outView) *outView = newTexData.view.Get();

        texturesCache.emplace(nameId, newTexData);
        return { nameId, true };
    }

    S_CATCH{ S_THROW_EX("STextureManagerDX11::LoadTexture('", texPath.string().c_str(), "')"); }

    return { 0, false };
}

void STextureManagerDX11::ClearCache(IWorld* world)
{/*
    S_TRY

    if (world)
    {
        std::set<LcPath> aliveTexList;
        const auto& visuals = world->GetVisuals();
        for (const auto& visual : visuals)
        {
            if (auto texComp = visual->GetTextureComponent())
            {
                aliveTexList.insert(texComp->GetTexturePath());
            }
        }

        if (auto fontManagerPtr = world->GetFontManager())
        {
            for (const auto& fontPath : *fontManagerPtr->GetFontsList())
            {
                aliveTexList.insert(fontPath);
            }
        }

        std::set<LcPath> eraseTexList;
        for (auto tex : texturesCache)
        {
            if (aliveTexList.find(tex.first) == aliveTexList.end())
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

    S_CATCH{ S_THROW("STextureManagerDX11::ClearCache()") }*/
}

#endif // WIN32
