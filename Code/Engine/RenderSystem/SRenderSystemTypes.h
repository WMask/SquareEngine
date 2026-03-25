/***************************************************************************
* SRenderSystemTypes.h
*/

#pragma once

#include <cstdint>


/** Max sprites count in one batch */
static const std::uint32_t MaxInstancedSpritesCount = 512u;

/** Render system stats */
struct SRSStats
{
	std::int32_t numTextures;
	std::int32_t numTilemaps;
	std::int32_t numFonts;
};

/** Matrix buffer */
struct SSingleMatrixBuffer
{
	SMatrix4 mat;
};

/** Render settings */
struct SSettingsBuffer
{
	SVector4 worldTint;
};

/** Sprite flags */
struct SSpriteFlagsBuffer
{
	std::int32_t bHasAnimation;
	std::int32_t bHasColor;
	std::int32_t bHasCustomUV;
	std::int32_t bHasTexture;
};
