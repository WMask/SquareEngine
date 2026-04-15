/***************************************************************************
* SUtilsWindows.h
*/

#ifdef WIN32

#include "RenderSystem/Windows/SUtilsWindows.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <wrl.h>
#include <vector>
#include <cmath>
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")


bool GetHardwareAdapter(ComPtr<IDXGIFactory2>& factory, ComPtr<IDXGIAdapter1>& outAdapter)
{
    outAdapter.Reset();

    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(factory.As(&factory6)))
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
                adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(adapter->GetDesc1(&desc)))
            {
                return false;
            }

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            DebugMsgW(L"GetHardwareAdapter(1): Selected Direct3D Adapter: VID:%04X, PID:%04X Name:%ls\n",
                desc.VendorId, desc.DeviceId, desc.Description);
            break;
        }
    }

    if (!adapter)
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(factory->EnumAdapters1(
                adapterIndex, adapter.ReleaseAndGetAddressOf()));
                adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(adapter->GetDesc1(&desc)))
            {
                return false;
            }

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            DebugMsgW(L"GetHardwareAdapter(2): Selected Direct3D Adapter: VID:%04X, PID:%04X Name:%ls\n",
                desc.VendorId, desc.DeviceId, desc.Description);
            break;
        }
    }

    outAdapter.Attach(adapter.Detach());
    return outAdapter;
}

bool FindDisplayMode(ComPtr<IDXGIFactory2>& factory, std::int32_t width, std::int32_t height, std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode)
{
    if (!outMode) return false;

    outMode->RefreshRate.Numerator = 1;
    outMode->RefreshRate.Denominator = 1;
    outMode->Format = DXGI_FORMAT_UNKNOWN;

    ComPtr<IDXGIAdapter1> adapter;
    if (!GetHardwareAdapter(factory, adapter))
    {
        return false;
    }

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

    return (outMode->Format != DXGI_FORMAT_UNKNOWN);
}

void MakeWindowAssociation(HWND hWnd)
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
