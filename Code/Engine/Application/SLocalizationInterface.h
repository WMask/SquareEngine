/***************************************************************************
* SLocalizationInterface.h
*/

#pragma once

#include "Application/SApplicationTypes.h"

#include <entt/entt.hpp>
#include <filesystem>
#include <string>


namespace SConst
{
	constexpr std::string_view DemoTextKey = "demo_text";
	constexpr std::string_view ApplyTextKey = "apply_text";
	constexpr std::string_view ControlsTextKey = "controls_text";
	constexpr std::string_view DefaultTextKey = "default_text";
	constexpr std::string_view PBRTextKey = "pbr_text";
	constexpr std::string_view ToggleTextKey = "toggle_text";
	constexpr std::string_view FpsTextKey = "fps_text";
	constexpr std::string_view FpsFmtKey = "fps_fmt";
	constexpr std::string_view PolyTextKey = "poly_text";
	constexpr std::string_view PolyFmtKey = "poly_fmt";
	constexpr std::string_view BackLightTextKey = "back_light_text";
	constexpr std::string_view BackLightFmtKey = "back_light_fmt";
	constexpr std::string_view ReflectionTextKey = "reflection_text";
	constexpr std::string_view ReflectionFmtKey = "reflection_fmt";
}


/** localized text key */
using STextID = std::uint32_t;

/** Localization interface */
class ILocalization
{
public:
	/**
	* Subscribe to get changes of culture. */
	entt::delegate<void(std::string, const SAppContext&)> onCultureChanged;


public:
	//
	virtual ~ILocalization() {}
	//
	virtual void Init(const SAppContext* context) = 0;
	// add new culture
	virtual void AddCulture(const SPath& filePath) = 0;
	// culture like "en", "es", "fr"
	virtual bool SetCulture(const std::string& culture) = 0;
	// set value
	virtual void Set(const std::string_view& key, const std::wstring& value) = 0;
	// get localized string by key name
	virtual std::pair<std::wstring, bool> Get(const std::string_view& key) const = 0;
	// get localized string by key id
	virtual std::pair<std::wstring, bool> Get(STextID key) const = 0;
	// get current culture's name
	virtual const std::string& GetCulture() const = 0;
};

using TLocalizationPtr = std::unique_ptr<ILocalization>;
