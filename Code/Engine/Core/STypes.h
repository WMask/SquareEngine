/***************************************************************************
* STypes.h
*/

#pragma once

#include "Core/SMathTypes.h"

#include <string>
#include <optional>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <vector>


namespace SConst
{
	static const std::uint32_t MaxWeightsPerVertex = 4u;
	static const std::uint32_t AnimationFramesPerSecond = 30u;
	static const float AnimationFrameDelta = 1.0f / AnimationFramesPerSecond;
}

/** Render system type */
enum class SRSType
{
	DX11,
	DX12,
	Vulkan,
	Metal
};

/** Bytes array */
using SBytes = std::vector<std::uint8_t>;


/***************************************************************************
* Uncopyable type
*/
class SUncopyable
{
public:
	//
	SUncopyable() {}

private:
	//
	SUncopyable(const SUncopyable&) = delete;
	//
	SUncopyable& operator=(const SUncopyable&) = delete;
};


/***************************************************************************
* Range class. Clamps value to specified range.
*/
template<typename T, const T& minValue, const T& maxValue>
class TRange
{
public:
	TRange() : value{} {}
	//
	TRange(const TRange& ref) : value(ref.value) {}
	//
	TRange(T inValue) : value(std::clamp(inValue, minValue, maxValue)) {}
	//
	inline TRange& operator = (const TRange& ref) { value = ref.value; return *this; }
	//
	inline operator T () const { return value; }
	//
	inline T* get() const { return const_cast<T*>(&value); }

private:
	//
	T value;
};

/**
* Check bounds (compile time) */
template <typename T>
constexpr bool InRange(T value, T minValue, T maxValue)
{
	return (value >= minValue && value <= maxValue);
}


/***************************************************************************
* Mesh
*/

/** Id in texture manager */
using STexID = std::uint32_t;

/** Static mesh vertex */
struct SVertex
{
	SVector3 pos;
	SVector3 norm;
	SVector3 tangent;
	SVector2 uv;
};

/** Skeletal mesh vertex */
struct SBlendVertex : public SVertex
{
	std::uint32_t indices[SConst::MaxWeightsPerVertex];
	//
	float weights[SConst::MaxWeightsPerVertex];
};

/** Mesh material base */
struct SMaterialBase
{
	std::uint32_t firstIndex;
	//
	std::uint32_t numVertices;
	//
	std::uint32_t numIndices;
};

/** Mesh material */
struct SMaterial : public SMaterialBase
{
	// rgba
	std::filesystem::path baseTexture;
	// tangent space
	std::filesystem::path normTexture;
	// r - roughness, g - metallic, b - AO
	std::filesystem::path rmaTexture;
	// rgb - emissive
	std::filesystem::path emiTexture;
};

/** Mesh material */
struct SMeshMaterial : public SMaterialBase
{
	STexID baseTexId;
	//
	STexID normTexId;
	//
	STexID rmaTexId;
	//
	STexID emiTexId;
};

/** Id in mesh manager */
using SMeshID = std::uint32_t;

/** Id in mesh manager */
using SAnimID = std::uint32_t;

/** Static mesh */
struct SMesh
{
	// generated from mesh name
	SMeshID id{};
	//
	std::string name;
	//
	std::vector<SMaterial> materials;
	//
	std::vector<SVertex> vertices;
	//
	std::vector<std::uint16_t> indices16;
	//
	std::vector<std::uint32_t> indices32;
};

/** Baked skeletal animation frame */
struct SBakedAnimationFrame
{
	std::vector<SMatrix4> mTrans;
	//
	std::vector<SVector3> trans;
	//
	std::vector<SQuat> rotation;
	//
	std::vector<SVector3> scale;
	// in seconds
	float time;
};

/** Baked skeletal animation */
struct SBakedSkeletalAnimation
{
	// generated from FBX file name
	SAnimID id{};
	// generated from mesh name
	SMeshID meshId{};
	//
	std::string name;
	//
	std::uint32_t framesPerSecond;
	// in seconds
	float duration;
	//
	std::vector<SBakedAnimationFrame> frames;
};

using TBonesMap = std::unordered_map<std::string, std::uint32_t>;

/** Skeletal mesh */
struct SSkeletalMesh
{
	// generated from mesh name
	SMeshID id{};
	//
	std::string name;
	//
	STransform transform;
	//
	TBonesMap bones;
	//
	std::vector<SMaterial> materials;
	//
	std::vector<SBlendVertex> vertices;
	//
	std::vector<std::uint16_t> indices16;
	//
	std::vector<std::uint32_t> indices32;
};

/** Meshes group id in mesh manager */
using SGroupID = std::uint32_t;

/** Mesh instance */
struct SMeshInstance
{
	//
	SMeshID id;
	//
	SGroupID group;
	//
	STransform transform;
};


/***************************************************************************
* Color
*/

// Color
struct SColor3
{
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;

	bool operator==(const SColor3&) const = default;
};

// Color
struct SColor3F
{
	float r;
	float g;
	float b;

	bool operator==(const SColor3F&) const = default;
};

// Color with alpha
struct SColor4F : public SColor3F
{
	float a;

	bool operator==(const SColor4F&) const = default;
	inline operator const float*() const { return &r; }
};

namespace SConst
{
	static const SColor3 White3 = SColor3{ 255, 255, 255 };
	static const SColor3F White3F = SColor3F{ 1.0f, 1.0f, 1.0f };
	static const SColor4F White4F = SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f };
	static const SColor4F Black4F = SColor4F{ 0.0f, 0.0f, 0.0f, 1.0f };
}

namespace SConvert
{
	inline SColor4F FromSColor3(const SColor3& color)
	{
		return SColor4F {
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f,
			1.0f
		};
	}

	inline SVector3 ToVector3(const SColor3F& color)
	{
		return SVector3 {
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f
		};
	}

	inline SVector4 ToVector4(const SColor3& color)
	{
		return SVector4 {
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f,
			1.0f
		};
	}

	inline SColor4F ToColor4(const SVector4& v)
	{
		return SColor4F{ v.x, v.y, v.z, v.w };
	}

	inline SColor4F ToColor4(const SColor3F& color)
	{
		return SColor4F{ color.r, color.g, color.b, 1.0f };
	}

	inline SColor4F ToColor4(const SColor3& color)
	{
		return SColor4F{
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f,
			1.0f
		};
	}

#if USING_DXMATH

	inline DirectX::XMVECTOR ToXVector4(const SColor3F& c)
	{
		return DirectX::XMVectorSet(c.r, c.g, c.b, 1.0f);
	}

	inline DirectX::XMVECTOR ToXVector4(const SColor4F& c)
	{
		return DirectX::XMVectorSet(c.r, c.g, c.b, c.a);
	}

	inline SColor3F ToColor3(const SColor3& c)
	{
		return SColor3F {
			static_cast<float>(c.r) / 255.0f,
			static_cast<float>(c.g) / 255.0f,
			static_cast<float>(c.b) / 255.0f
		};
	}

	inline SColor3F ToColor3(DirectX::XMVECTOR v)
	{
		return SColor3F{ DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v) };
	}

	inline SColor4F ToColor4(DirectX::XMVECTOR v)
	{
		return SColor4F{ DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v), DirectX::XMVectorGetW(v) };
	}

#endif
}
