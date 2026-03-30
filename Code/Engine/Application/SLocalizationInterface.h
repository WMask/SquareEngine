/***************************************************************************
* SLocalizationInterface.h
*/

#pragma once

#include "Application/SApplicationTypes.h"

#include <entt/entt.hpp>
#include <filesystem>
#include <string>


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
	virtual void AddCulture(const std::filesystem::path& filePath) = 0;
	// culture like "en", "es", "fr"
	virtual bool SetCulture(const std::string& culture) = 0;
	// set value
	virtual void Set(const std::string& key, const std::wstring& value) = 0;
	// get localized string by key name
	virtual std::pair<std::wstring, bool> Get(const std::string_view& key) const = 0;
	// get localized string by key id
	virtual std::pair<std::wstring, bool> Get(STextID key) const = 0;
	// get current culture's name
	virtual const std::string& GetCulture() const = 0;
	// make text key id from key name
	virtual STextID MakeId(const std::string_view& key) const = 0;
};

using TLocalizationPtr = std::unique_ptr<ILocalization>;
