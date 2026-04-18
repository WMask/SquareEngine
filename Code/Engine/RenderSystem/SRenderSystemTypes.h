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
	constexpr std::uint32_t MaxInstancedSpritesCount = 512u;
	/** Max meshes count in one batch */
	constexpr std::uint32_t MaxInstancedMeshesCount = 512u;
	/** Max lights count */
	constexpr std::uint32_t MaxLightsCount = 64u;
}

class IMeshManager;

/** Basic delegate */
using OnFinishedDelegate = std::function<void(bool)>;
/** Textures delegate */
using OnTexturesLoadedDelegate = std::function<void(bool, std::vector<STexID>&)>;
/** Textures delegate */
using OnAnimationsLoadedDelegate = std::function<void(bool, std::vector<SAnimID>&, IMeshManager&)>;
/** Mesh instances delegate */
using OnMeshInstancesLoadedDelegate = std::function<void(bool, const std::vector<SMeshInstance>&, IMeshManager&)>;
/** Skeletal mesh delegate */
using OnSkeletalMeshLoadedDelegate = std::function<void(bool, SMeshID, const STransform&, IMeshManager&)>;
/** Mesh delegate */
using OnMeshFinishedDelegate = std::function<void(bool, IMeshManager&)>;

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
	//
	SVector2 screenSize;
	//
	SVector2 padding;
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
	SPath path;
	bool bLoadFailed;
};

struct SCubemapData
{
	SBytes data;
	ECubemapType type;
	SPath path;
	bool bLoadFailed;
};

/** Texture lifetime policy */
class ITextureLifetime
{
public:
	//
	virtual std::shared_ptr<STextureBase> CreateTexture(const STextureData& data) = 0;
	//
	virtual std::shared_ptr<STextureBase> CreateCubemap(const SCubemapData& data) = 0;
	//
	virtual std::pair<SSize2, bool> GetTextureSize(STexID id) const = 0;
};

enum class EMeshType
{
	Static,
	Skeletal
};

struct SMeshBase
{
	virtual ~SMeshBase() {}
	//
	EMeshType type;
};

struct SMeshData
{
	// generated from file path
	SMeshID id{};
	//
	SPath path;
	//
	std::vector<SMesh> meshes;
	//
	std::vector<SMeshInstance> instances;
	//
	bool bLoadFailed;
};

struct SSkeletalMeshData
{
	// generated from file path
	SMeshID id{};
	//
	SSkeletalMesh mesh;
	//
	bool bLoadFailed;
};

struct SSkeletalAnimData
{
	// generated from file path
	SAnimID id{};
	//
	SBakedSkeletalAnimation anim;
	//
	bool bLoadFailed;
};


/***************************************************************************
* Texture manager interface
*/
class ITextureManager : public SUncopyable
{
public:
	/**
	* Request async texture loading and return id.
	* Loading may take some time, after that texture will be available at this id.
	* Render system skip rendering if texture not loaded yet.
	*/
	virtual STexID LoadTexture(const SPath& path) = 0;
	/**
	* Load textures and call delegate */
	virtual void PreloadTextures(const SPathList& paths) = 0;
	//
	virtual void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate) = 0;
	//
	virtual bool RemoveTexture(STexID id) = 0;
	/**
	* Get texture size in pixels */
	virtual std::pair<SSize2, bool> GetTextureSize(STexID id) const = 0;
	/**
	* Load cubemap from dds file */
	virtual void LoadCubemap(const SPath& path, ECubemapType type) = 0;
	//
	virtual bool RemoveCubemap(ECubemapType type) = 0;
};


/***************************************************************************
* Mesh manager interface
*/
class IMeshManager : public SUncopyable
{
public:
	/**
	* Load mesh scene instances and call delegate.
	* Loads meshes with material textures if instance's mesh not loaded yet.
	*/
	virtual void LoadStaticMeshInstances(const SPath& path, SGroupID groupId) = 0;
	//
	virtual void LoadStaticMeshInstances(const SPath& path, SGroupID groupId,
		OnMeshInstancesLoadedDelegate delegate) = 0;
	/**
	* Load meshes with material textures and call delegate */
	virtual void PreloadStaticMeshes(const SPath& path) = 0;
	//
	virtual void PreloadStaticMeshes(const SPath& path, OnMeshFinishedDelegate delegate) = 0;
	/**
	* Load skeletal mesh and call delegate */
	virtual void LoadSkeletalMesh(const SPath& path) = 0;
	//
	virtual void LoadSkeletalMesh(const SPath& path, OnSkeletalMeshLoadedDelegate delegate) = 0;
	/**
	* Load animations and call delegate */
	virtual void PreloadAnimations(const SPathList& paths, SMeshID id) = 0;
	//
	virtual void PreloadAnimations(const SPathList& paths, SMeshID id,
		OnAnimationsLoadedDelegate delegate) = 0;
};

/** Mesh lifetime policy */
class IMeshLifetime
{
public:
	//
	virtual std::shared_ptr<SMeshBase> CreateMesh(const SMesh& data) = 0;
	//
	virtual std::shared_ptr<SMeshBase> CreateSkeletalMesh(const SSkeletalMesh& data) = 0;
	//
	virtual bool GetBones(SMeshID id, TBonesMap& bones) const = 0;
};
