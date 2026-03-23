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
