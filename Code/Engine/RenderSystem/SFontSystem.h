/***************************************************************************
* SFontSystem.h
*/

#pragma once

#include "RenderSystem/SRenderSystemTypes.h"
#include "Core/SUtils.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <array>
#include <map>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
# pragma warning(disable : 5046)
#endif


/** Font glyph */
struct SGlyph
{
	// position on texture in pixels
	SVector2 pos;
	// size on texture in pixels
	SSize2F size;
};


/***************************************************************************
* Font class
*/
class SFont
{
public:
	//
	SFont() : fontSize(0) {}
	//
	void Load(const std::filesystem::path& jsonPath, STexID textureId);
	//
	std::pair<SGlyph, bool> FindGlyph(wchar_t glyphCode) const;
	//
	inline bool HasGlyph(wchar_t glyphCode) const { return glyphs.find(glyphCode) != glyphs.end(); }
	//
	inline size_t GetGlyphsCount() const { return glyphs.size(); }
	//
	inline std::wstring_view GetDisplayName() const { return displayName; }
	//
	inline float GetFontSizeF() const { return static_cast<float>(fontSize); }
	//
	inline unsigned int GetFontSize() const { return fontSize; }
	//
	inline STexID GetTextureId() const { return texId; }


protected:
	//
	std::map<wchar_t, SGlyph> glyphs;
	//
	std::wstring displayName;
	//
	STexID texId;
	//
	unsigned int fontSize;

};


/***************************************************************************
* Font manager
*/
class SFontManager : public SUncopyable
{
public:
	//
	SFontManager() {}
	//
	virtual ~SFontManager() {}
	//
	void AddFont(const std::filesystem::path& jsonPath, const std::string_view& fontName, STexID textureId);
	//
	std::pair<STexID, bool> GetTextureId(const std::string_view& fontName) const;
	//
	std::pair<SGlyph, bool> FindGlyph(const std::string_view& fontName, wchar_t glyphCode) const;
	// outTextSize - full string size in pixels
	bool FindGlyphs(const std::string_view& fontName, const std::wstring_view& text,
		std::vector<SGlyph>& outGlyphs, SSize2F* outTextSize) const;


protected:
	// key - font name
	std::unordered_map<std::string, SFont> fonts;

};
