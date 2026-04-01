/***************************************************************************
* SLocalization.h
*/

#pragma once

#include "Application/SLocalizationInterface.h"
#include "Core/STypes.h"

#include <map>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
# pragma warning(disable : 4275)
#endif


/** Localization file */
class SLocalization : public SUncopyable
{
public:
	//
	SLocalization() {}
	//
	~SLocalization() {}
	//
	void Load(const std::filesystem::path& filePath);
	//
	void Set(const std::string_view& key, const std::wstring& value);
	//
	std::pair<std::wstring, bool> Get(STextID key) const;
	//
	std::pair<std::wstring, bool> Get(const std::string_view& key) const;
	//
	const std::string& GetCulture() const { return culture; }


protected:
	//
	std::string culture;
	//
	std::map<STextID, std::wstring> entries;

};


/** Localization manager */
class SLocalizationManager : public ILocalization
{
public:
	SLocalizationManager() : context(nullptr) {}
	//
	virtual ~SLocalizationManager() {}


public:
	//
	virtual void Init(const SAppContext* inContext) override { context = inContext; }
	//
	virtual void AddCulture(const std::filesystem::path& filePath) override;
	//
	virtual bool SetCulture(const std::string& culture) override;
	//
	virtual void Set(const std::string_view& key, const std::wstring& value) override;
	//
	virtual std::pair<std::wstring, bool> Get(STextID key) const override;
	//
	virtual std::pair<std::wstring, bool> Get(const std::string_view& key) const override;
	//
	virtual const std::string& GetCulture() const override { return curCulture; }


protected:
	//
	std::map<std::string, std::shared_ptr<SLocalization>> cultures;
	//
	std::string curCulture;
	//
	const SAppContext* context;

};
