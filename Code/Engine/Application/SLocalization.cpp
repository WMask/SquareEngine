/***************************************************************************
* SLocalization.cpp
*/

#include "Application/SLocalization.h"
#include "Application/SApplicationModule.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;


namespace SApplication
{
    TLocalizationPtr CreateLocalization(const SAppContext& context)
    {
        auto loc = std::make_unique<SLocalizationManager>();
        loc->Init(&context);
        return loc;
    }
}

void SLocalization::Load(const std::filesystem::path& filePath)
{
    S_TRY

    auto fileText = ReadTextFile(filePath);

    json locFile = json::parse(fileText.c_str());
    culture = locFile["culture"].get<std::string>();

    for (auto entry : locFile["entries"])
    {
        std::string key = entry["key"].get<std::string>();
        std::string utf8enc = entry["value"].get<std::string>();
        std::wstring value = FromUtf8(utf8enc);
        Set(key, value);
    }

    S_CATCH {
        std::string msg = ToUtf8(filePath.c_str());
        S_THROW_EX("SLocalization::Load('", msg.c_str(), "')");
    }
}

void SLocalization::Set(const std::string_view& key, const std::wstring& value)
{
    STextID id = ResourceID<STextID>(key);
    entries.insert_or_assign(id, value);
}

std::pair<std::wstring, bool> SLocalization::Get(STextID key) const
{
    auto it = entries.find(key);
    if (it != entries.end())
    {
        return { it->second, true };
    }

    return { L"", false };
}

std::pair<std::wstring, bool> SLocalization::Get(const std::string_view& key) const
{
    STextID id = ResourceID<STextID>(key);
    auto it = entries.find(id);
    if (it != entries.end())
    {
        return { it->second, true };
    }

    return { L"", false };
}

void SLocalizationManager::AddCulture(const std::filesystem::path& filePath)
{
    S_TRY

    auto newCulture = std::make_shared<SLocalization>();
    newCulture->Load(filePath);

    auto newCultureName = newCulture->GetCulture();
    bool bCultureChanged = cultures.empty();

    cultures.emplace(newCultureName, newCulture);

    if (bCultureChanged)
    {
        curCulture = newCultureName;

        if (context && onCultureChanged) onCultureChanged(curCulture, *context);
    }

    S_CATCH{ S_THROW("SLocalizationManager::AddCulture()"); }
}

bool SLocalizationManager::SetCulture(const std::string& inCulture)
{
    auto it = cultures.find(curCulture);
    if (it != cultures.end() &&
        curCulture != inCulture)
    {
        curCulture = inCulture;
        if (context && onCultureChanged) onCultureChanged(curCulture, *context);

        return true;
    }

    return false;
}

void SLocalizationManager::Set(const std::string_view& key, const std::wstring& value)
{
    auto it = cultures.find(curCulture);
    if (it != cultures.end())
    {
        it->second->Set(key, value);
    }
}

std::pair<std::wstring, bool> SLocalizationManager::Get(STextID key) const
{
    auto cultureIt = cultures.find(curCulture);
    if (cultureIt != cultures.end())
    {
        return cultureIt->second->Get(key);
    }

    return { L"", false };
}

std::pair<std::wstring, bool> SLocalizationManager::Get(const std::string_view& key) const
{
    auto cultureIt = cultures.find(curCulture);
    if (cultureIt != cultures.end())
    {
        return cultureIt->second->Get(key);
    }

    return { L"", false };
}
