/***************************************************************************
* SWindowsUtils.h
*/

#pragma once

#include "Core/SMathTypes.h"

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


namespace SConst
{
	static const DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
}


/** Find display mode */
bool SFindDisplayMode(std::int32_t width, std::int32_t height, std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode);

/** Make window association to enable fullscreen mode on Alt + Enter */
void SMakeWindowAssociation(HWND hWnd);
