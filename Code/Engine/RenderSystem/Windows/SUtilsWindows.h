/***************************************************************************
* SUtilsWindows.h
*/

#pragma once

#include "Core/SMathTypes.h"

#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <mutex>

using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Wrappers::CriticalSection;


namespace SConst
{
	static const DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	static const DXGI_FORMAT DefaultHDRBackBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
	static const DXGI_FORMAT DefaultSDRRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT DefaultHDRRenderTargetFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	static const DXGI_COLOR_SPACE_TYPE DefaultSDRColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	static const DXGI_COLOR_SPACE_TYPE DefaultHDRColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
	static const UINT DefaultBackBufferCount = 2u;
}

using TLockGuard = std::lock_guard<std::mutex>;

/** Get hardware adapter */
bool GetHardwareAdapter(ComPtr<IDXGIFactory2>& factory, ComPtr<IDXGIAdapter1>& outAdapter);

/** Find display mode */
bool FindDisplayMode(ComPtr<IDXGIFactory2>& factory, std::int32_t width, std::int32_t height,
	std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode);

/** Make window association to enable fullscreen mode on Alt + Enter */
void MakeWindowAssociation(HWND hWnd);
