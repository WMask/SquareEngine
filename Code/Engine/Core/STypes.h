/***************************************************************************
* STypes.h
*/

#pragma once

#include <string>
#include <optional>


/** Render system */
enum class SRSType {
	DX11,
	DX12,
	Vulkan
};

/** Any value container */
struct SAny
{
	enum class SAnyType { StringAny, FloatAny, IntAny, BoolAny };
	//
	SAny() : fValue(0.0f), iValue(0) {}
	SAny(const std::string& value) : type(SAnyType::StringAny), sValue(value), fValue(0.0f), iValue(0) {}
	SAny(float value) : type(SAnyType::FloatAny), fValue(value), iValue(0) {}
	SAny(int value) : type(SAnyType::IntAny), fValue(0.0f), iValue(value) {}
	SAny(bool value) : type(SAnyType::BoolAny), fValue(0.0f), iValue(value ? 1 : 0) {}
	//
	inline bool bValue() const { return (iValue == 1); }
	//
	std::string             sValue;
	float                   fValue{};
	std::int32_t            iValue{};
	std::optional<SAnyType> type;
};
