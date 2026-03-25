/***************************************************************************
* SLocalization.cpp
*/

#include "Application/SLocalization.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

static const wchar_t* default_value = L"default_value";


void SLocalization::Load(const char* filePath)
{
    S_TRY

    auto fileText = ReadTextFile(filePath);

    json locFile = json::parse(fileText);
    culture = locFile["culture"].get<std::string>();

    for (auto entry : locFile["entries"])
    {
        std::string key = entry["key"].get<std::string>();
        std::string utf8enc = entry["value"].get<std::string>();
        std::wstring value = FromUtf8(utf8enc);

        entries[key] = value;
    }

    S_CATCH{ S_THROW_EX("SLocalization::Load('", filePath, "')"); }
}

std::wstring SLocalization::Get(const char* key) const
{
    auto it = entries.find(key);
    return (it == entries.end()) ? default_value : it->second;
}

void SLocalizationManager::AddCulture(const char* filePath)
{
    S_TRY

    auto newCulture = std::make_shared<SLocalization>();
    newCulture->Load(filePath);

    auto cultureName = newCulture->GetCulture();
    bool cultureChanged = cultures.empty();

    cultures.emplace(cultureName, newCulture);

    if (cultureChanged)
    {
        culture = cultureName;

        if (context && onCultureChanged) onCultureChanged(cultureName, *context);
    }

    S_CATCH{ S_THROW("SLocalizationManager::AddCulture()"); }
}

bool SLocalizationManager::SetCulture(const char* inCulture)
{
    auto it = cultures.find(culture);
    if (it != cultures.end() &&
        culture != inCulture)
    {
        culture = inCulture;
        if (context && onCultureChanged) onCultureChanged(culture, *context);
        return true;
    }

    return false;
}

void SLocalizationManager::Set(const char* key, const wchar_t* value)
{
    auto it = cultures.find(culture);
    if (it != cultures.end() && key && value)
    {
        it->second->Set(key, value);
    }
}

std::wstring SLocalizationManager::Get(const char* key) const
{
    auto it = cultures.find(culture);
    return (it == cultures.end()) ? default_value : it->second->Get(key);
}
