/***************************************************************************
* SRenderSystemTypes.h
*/

#pragma once

#include "Core/STypes.h"

#include <cstdint>
#include <filesystem>
#include <functional>


/** Max sprites count in one batch */
static const std::uint32_t MaxInstancedSpritesCount = 512u;

/** Id in texture manager */
using STexID = std::uint32_t;
/** Textures delegate */
using OnTexturesLoadedDelegate = std::function<void(std::vector<std::filesystem::path>&)>;
/** Mesh instances delegate */
using OnMeshInstancesLoadedDelegate = std::function<void(std::filesystem::path, const std::vector<SMeshInstance>&)>;
/** Meshes delegate */
using OnMeshesLoadedDelegate = std::function<void(std::filesystem::path)>;

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
