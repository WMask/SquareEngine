/***************************************************************************
* SRenderSystemTypes.h
*/

#pragma once

#include "Core/STypes.h"

#include <cstdint>
#include <filesystem>
#include <functional>


namespace SConst
{
	/** Max sprites count in one batch */
	static const std::uint32_t MaxInstancedSpritesCount = 512u;
	/** Max meshes count in one batch */
	static const std::uint32_t MaxInstancedMeshesCount = 512u;
	/** Max lights count */
	static const std::uint32_t MaxLightsCount = 64u;
}

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

enum ECubemapType : int
{
	// diffuse ambient light (radiance)
	Diffuse,
	// specular reflection (irradiance)
	Specular
};

namespace SConst
{
	static std::string_view GetNameByType(ECubemapType type)
	{
		static const std::string_view diffuseName = "Diffuse";
		static const std::string_view specularName = "Specular";

		return (type == ECubemapType::Diffuse) ? diffuseName : specularName;
	}
}

/** Render settings */
struct SSettingsBuffer
{
	SVector4 worldTint;
	//
	SVector4 cameraPos;
	//
	SVector4 viewDir;
	//
	std::uint32_t bHasDiffuseCubemap;
	//
	std::uint32_t bHasSpecularCubemap;
	//
	float diffuseAmount;
	//
	float specularAmount;
};

/** Lights settings */
struct SLightsBuffer
{
	// w - light type: <0.5 - directional, >0.5 - point
	SVector4 lightVec[SConst::MaxLightsCount];
	// w - distance if point light type
	SVector4 lightColor[SConst::MaxLightsCount];
	//
	std::uint32_t numLights{};
	//
	float padding[3];
};

/** Material flags */
struct SMaterialBuffer
{
	std::int32_t bHasBaseTexture;
	std::int32_t bHasNormTexture;
	std::int32_t bHasRMATexture; // r - roughness, g - metallic, b - AO
	std::int32_t bHasEmiTexture; // rgb - emissive
};

/** Sprite flags */
struct SSpriteFlagsBuffer
{
	std::int32_t bHasAnimation;
	std::int32_t bHasColor;
	std::int32_t bHasCustomUV;
	std::int32_t bHasTexture;
};

/** Align buffer size */
template<typename T>
constexpr std::uint32_t Align16()
{
	return (sizeof(T) + 15) & ~15;
}
