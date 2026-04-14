/***************************************************************************
* SRenderSystemTypes.h
*/

#pragma once

#include "Core/SMath.h"

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

/** Textures delegate */
using OnTexturesLoadedDelegate = std::function<void(std::vector<STexID>&)>;
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
	SVector4 cameraPos;
	//
	SVector4 viewDir;
	// for black screen effect
	SColor4F globalTint = SConst::White4F;
	// for 3d meshes
	SColor4F backLight = SConst::White4F / 3.0f;
	// apply correction on final pbr color of 3d meshes
	SColor4F pbrGammaCorrection = SConst::White4F;
};

namespace SConst
{
	static const SSettingsBuffer DefaultRenderSettings{};
}

/** Cubemaps settings */
struct SCubemapsBuffer
{
	std::uint32_t bHasDiffuseCubemap;
	//
	std::uint32_t bHasSpecularCubemap;
	//
	float diffuseAmount = 1.0f;
	//
	float specularAmount = 1.0f;
	//
	std::uint32_t maxCubemapMipLevels = 1u;
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
struct SMaterialFlagsBuffer
{
	std::uint32_t bHasBaseTexture;
	std::uint32_t bHasNormTexture; // rgb - tangent space normal
	std::uint32_t bHasRMATexture;  // r - roughness, g - metallic, b - AO
	std::uint32_t bHasEmiTexture;  // rgb - emissive
};

/** Sprite flags */
struct SSpriteFlagsBuffer
{
	std::uint32_t bHasAnimation;
	std::uint32_t bHasColor;
	std::uint32_t bHasCustomUV;
	std::uint32_t bHasTexture;
};

/** Align buffer size */
template<typename T>
constexpr std::uint32_t Align16()
{
	return (sizeof(T) + 15) & ~15;
}

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

struct STextureBase
{
	virtual ~STextureBase() {}
};

struct STextureData
{
	SBytes data;
	SSize2 texSize;
	STexID id;
};

struct SCubemapData
{
	SBytes data;
	ECubemapType type;
};

/** Texture lifetime policy */
class ITextureLifetime
{
public:
	//
	virtual std::shared_ptr<STextureBase> CreateTexture(const STextureData& data) = 0;
	//
	virtual std::shared_ptr<STextureBase> CreateCubemap(const SCubemapData& data) = 0;
};

struct SMeshBase
{
	virtual ~SMeshBase() {}
};

struct SMeshData
{
	// generated from fbx path
	SMeshID id{};
	//
	std::vector<SMesh> meshes;
	//
	std::vector<SMeshInstance> instances;
};

/** Mesh lifetime policy */
class IMeshLifetime
{
public:
	//
	virtual std::shared_ptr<SMeshBase> CreateMesh(const SMesh& data) = 0;
	//
	virtual std::shared_ptr<SMeshBase> CreateSkeletalMesh(const SMesh& data) = 0;
};
