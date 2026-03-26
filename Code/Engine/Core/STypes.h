/***************************************************************************
* STypes.h
*/

#pragma once

#include "Core/SMathTypes.h"

#include <string>
#include <optional>
#include <algorithm>
#include <vector>


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
	return (value >= minValue && value < maxValue);
}


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
	static const SColor3 OneSColor3 = SColor3{ 255, 255, 255 };
	static const SColor3F OneSColor3F = SColor3F{ 1.0f, 1.0f, 1.0f };
	static const SColor4F OneSColor4F = SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f };
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

	inline SVector4 ToVector4(const SColor3& color)
	{
		return SVector4 {
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f,
			1.0f
		};
	}
}
