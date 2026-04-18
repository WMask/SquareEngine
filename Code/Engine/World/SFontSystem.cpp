/***************************************************************************
* SFontSystem.cpp
*/

#include "World/SFontSystem.h"
#include "Application/SLocalizationInterface.h"
#include "Core/SException.h"
#include "Core/SUtils.h"

#include <string>
#include <vector>
#include <codecvt>

#include "nlohmann/json.hpp"
using json = nlohmann::json;


void SFont::Load(const SPath& jsonPath, STexID textureId)
{
	S_TRY

	auto fullJsonText = ReadTextFile(jsonPath);
	if (fullJsonText.length() == 0)
	{
		throw std::exception("Cannot read json");
	}

	json font = json::parse(fullJsonText);

	fontSize = font["fontSize"].get<std::uint32_t>();
	culture = font["culture"].get<std::string>();
	name = font["displayName"].get<std::string>();
	texId = textureId;

	json glyphsArray = font["glyphs"];
	if (glyphsArray.is_array())
	{
		for (auto& glyphIt : glyphsArray.items())
		{
			auto glyphObj = glyphIt.value();
			if (glyphObj["glyph"].is_string() &&
				glyphObj["pos"].is_object() &&
				glyphObj["size"].is_object())
			{
				std::string utf8Glyph = glyphObj["glyph"].get<std::string>();
				if (utf8Glyph.length() == 2)
				{
					if (utf8Glyph[0] == '\'') utf8Glyph = "\"";
					if (utf8Glyph[0] == '\\') utf8Glyph = "\\";
				}

				SGlyph glyph{};
				glyph.pos.x = glyphObj["pos"]["x"].get<float>();
				glyph.pos.y = glyphObj["pos"]["y"].get<float>();
				glyph.size.width = glyphObj["size"]["x"].get<float>();
				glyph.size.height = glyphObj["size"]["y"].get<float>();
				std::wstring glyphName = FromUtf8(utf8Glyph);
				glyphs.insert({ glyphName[0], glyph });
			}
		}
	}

	S_CATCH{ S_THROW("SFont::Load()") }
}

std::pair<SGlyph, bool> SFont::FindGlyph(wchar_t glyphCode) const
{
	auto glyphIt = glyphs.find(glyphCode);
	if (glyphIt != glyphs.end())
	{
		return { glyphIt->second, true };
	}

	return { SGlyph{}, false };
}

SFontID SFontSystem::AddFont(const SPath& jsonPath, STexID textureId)
{
	SFontID id = 0u;

	S_TRY

	SFont newFont;
	newFont.Load(jsonPath, textureId);
	id = hasher(newFont.GetName());
	fonts.insert({ id, newFont });

	S_CATCH{ S_THROW("SFontSystem::AddFont()") }

	return id;
}

std::pair<STexID, bool> SFontSystem::GetTextureId(SFontID fontId, const std::string& culture) const
{
	auto allCultures = fonts.equal_range(fontId);
	for (auto it = allCultures.first; it != allCultures.second; ++it)
	{
		if (it->second.GetCulture() == culture)
		{
			return { it->second.GetTextureId(), true};
		}
	}

	return { 0u, false };
}

std::pair<SGlyph, bool> SFontSystem::FindGlyph(SFontID fontId, wchar_t charCode, float* outLineHeight) const
{
	const auto culture = context->text->GetCulture();
	auto allCultures = fonts.equal_range(fontId);
	for (auto it = allCultures.first; it != allCultures.second; ++it)
	{
		if (it->second.GetCulture() == culture)
		{
			auto [glyph, bGlyphFound] = it->second.FindGlyph(charCode);
			if (bGlyphFound)
			{
				if (outLineHeight) *outLineHeight = static_cast<float>(it->second.GetSize());
				return { glyph, true };
			}
		}
	}

	return { SGlyph{}, false };
}

bool SFontSystem::FindGlyphs(SFontID fontId, const std::wstring& text, std::vector<SGlyph>* outGlyphs, SSize2F* outTextSize) const
{
	if (text.empty() || (!outGlyphs && !outTextSize)) return false;

	S_TRY

	const auto culture = context->text->GetCulture();
	const SFont* fontPtr = nullptr;
	auto allCultures = fonts.equal_range(fontId);
	for (auto it = allCultures.first; it != allCultures.second; ++it)
	{
		if (it->second.GetCulture() == culture)
		{
			auto [glyph, bGlyphFound] = it->second.FindGlyph(text[0]);
			if (bGlyphFound)
			{
				fontPtr = &it->second;
				break;
			}
		}
	}

	if (!fontPtr)
	{
		throw std::exception("Cannot find font");
	}

	if (outGlyphs)
	{
		outGlyphs->clear();
		outGlyphs->reserve(text.length());
	}

	float width = 0.0f;
	float height = static_cast<float>(fontPtr->GetSize());

	for (auto it = text.cbegin(); it != text.cend(); ++it)
	{
		auto [glyph, bGlyphFound] = fontPtr->FindGlyph(*it);
		if (!bGlyphFound)
		{
			throw std::exception("Cannot find glyph");
		}

		width += glyph.size.width;

		if (outGlyphs) outGlyphs->push_back(glyph);
	}

	if (outTextSize) *outTextSize = { width, height };

	S_CATCH{ S_THROW("SFontSystem::FindGlyphs()") }

	return true;
}

ILocalization* SFontSystem::GetLocale() const
{
	return context ? context->text : nullptr;
}
