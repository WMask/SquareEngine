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
    if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
    {
        throw std::exception("SMakeWindowAssociation(): Cannot create factory");
    }

    if (FAILED(factory->MakeWindowAssociation(hWnd, 0)))
    {
        throw std::exception("SMakeWindowAssociation(): Cannot make window association");
    }
}

#endif // WIN32
