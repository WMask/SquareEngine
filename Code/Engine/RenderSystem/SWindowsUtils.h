/***************************************************************************
* SWindowsUtils.h
*/

#pragma once

#include <d3d11.h>
#include <cstdint>


/** Find display mode */
bool SFindDisplayMode(std::int32_t width, std::int32_t height, std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode);

/** Make window association to enable fullscreen mode on Alt + Enter */
void SMakeWindowAssociation(HWND hWnd);
