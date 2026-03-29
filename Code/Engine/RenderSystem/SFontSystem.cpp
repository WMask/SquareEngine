/***************************************************************************
* SFontSystem.cpp
*/

#include "RenderSystem/SFontSystem.h"
#include "Core/SException.h"

#include <string>
#include <vector>
#include <codecvt>

#include "nlohmann/json.hpp"
using json = nlohmann::json;


void SFont::Load(const std::filesystem::path& jsonPath, STexID textureId)
{
	S_TRY

	auto fullJsonText = ReadTextFile(jsonPath);
	if (fullJsonText.length() == 0)
	{
		throw std::exception("Cannot read json");
	}

	json font = json::parse(fullJsonText);

	fontSize = font["fontSize"].get<unsigned int>();
	std::string utf8Name = font["displayName"].get<std::string>();
	displayName = FromUtf8(utf8Name);
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

void SFontManager::AddFont(const std::filesystem::path& jsonPath, const std::string_view& fontName, STexID textureId)
{
	S_TRY

	auto fontIt = fonts.find(fontName.data());
	if (fontIt != fonts.end())
	{
		fontIt->second.Load(jsonPath, textureId);
	}
	else
	{
		SFont newFont;
		newFont.Load(jsonPath, textureId);
		fonts.insert({ fontName.data(), newFont });
	}

	S_CATCH{ S_THROW("SFontManager::AddFont()") }
}

std::pair<STexID, bool> SFontManager::GetTextureId(const std::string_view& fontName) const
{
	auto fontIt = fonts.find(fontName.data());
	if (fontIt != fonts.end())
	{
		return { fontIt->second.GetTextureId(), true };
	}

	return { 0u, false };
}

std::pair<SGlyph, bool> SFontManager::FindGlyph(const std::string_view& fontName, wchar_t glyphCode) const
{
	auto fontIt = fonts.find(fontName.data());
	if (fontIt != fonts.end())
	{
		return fontIt->second.FindGlyph(glyphCode);
	}

	return { SGlyph{}, false };
}

bool SFontManager::FindGlyphs(const std::string_view& fontName, const std::wstring_view& text, std::vector<SGlyph>& outGlyphs, SSize2F* outTextSize) const
{
	S_TRY

	auto fontIt = fonts.find(fontName.data());
	if (fontIt == fonts.end())
	{
		throw std::exception("Cannot find font");
	}

	outGlyphs.clear();
	outGlyphs.reserve(text.length());

	float width = 0.0f, height = fontIt->second.GetFontSizeF();

	for (auto it(text.cbegin()); it != text.cend(); ++it)
	{
		auto [glyph, bFound] = fontIt->second.FindGlyph(*it);
		if (!bFound)
		{
			throw std::exception("Cannot find glyph");
		}

		width += glyph.size.width;
		outGlyphs.push_back(glyph);
	}

	if (outTextSize) *outTextSize = { width, height };

	return outGlyphs.size() == text.length();

	S_CATCH{ S_THROW_EX("SFontManager::FindGlyphs('", fontName.data(), "')") }

	return false;
}
