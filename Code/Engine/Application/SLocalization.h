/***************************************************************************
* SLocalization.h
*/

#pragma once

#include "Application/ApplicationModule.h"
#include "Application/SApplicationTypes.h"
#include <entt/entt.hpp>
#include <string>
#include <map>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
# pragma warning(disable : 4275)
#endif


/***************************************************************************
* Localization file
*/
class S_APPLICATION_API SLocalization
{
public:
	using TLocMap = std::map<std::string, std::wstring>;
	//
	SLocalization& operator=(const SLocalization&) = default;
	//
	SLocalization(const SLocalization&) = default;


public:
	SLocalization() {}
	//
	~SLocalization() {}
	//
	void Load(const char* filePath);
	//
	void Set(const char* key, const wchar_t* value) { entries[key] = value; }
	//
	std::wstring Get(const char* key) const;
	//
	const std::string& GetCulture() const { return culture; }


protected:
	std::string culture;
	//
	TLocMap entries;

};


/***************************************************************************
* Localization interface
*/
class ILocalizationManager
{
public:
	/**
	* Subscribe to get changes of culture. */
	entt::delegate<void(std::string, SAppContext)> onCultureChanged;


public:
	virtual ~ILocalizationManager() {}
	//
	virtual void Init(const SAppContext* context) = 0;
	// add new culture
	virtual void AddCulture(const char* filePath) = 0;
	// culture like "en", "es", "fr"
	virtual bool SetCulture(const char* culture) = 0;
	// set value
	virtual void Set(const char* key, const wchar_t* value) = 0;
	// get localized text entry
	virtual std::wstring Get(const char* key) const = 0;
	// get current culture's name
	virtual const std::string& GetCulture() const = 0;
};

using TLocalizationPtr = std::unique_ptr<ILocalizationManager>;


/***************************************************************************
* Localization manager
*/
class S_APPLICATION_API SLocalizationManager : public ILocalizationManager
{
public:
	SLocalizationManager() : context(nullptr) {}
	//
	virtual ~SLocalizationManager() {}


public:
	//
	virtual void Init(const struct SAppContext* inContext) override { context = inContext; }
	//
	virtual void AddCulture(const char* filePath) override;
	//
	virtual bool SetCulture(const char* culture) override;
	//
	virtual void Set(const char* key, const wchar_t* value) override;
	//
	virtual std::wstring Get(const char* key) const override;
	//
	virtual const std::string& GetCulture() const override { return culture; }


protected:
	//
	std::unordered_map<std::string, std::shared_ptr<SLocalization>> cultures;
	//
	std::string culture;
	//
	const SAppContext* context;

};
