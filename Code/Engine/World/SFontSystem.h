/***************************************************************************
* SFontSystem.h
*/

#pragma once

#include "World/SFontInterface.h"
#include "Application/SApplicationTypes.h"

#include <map>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
# pragma warning(disable : 5046)
#endif


/***************************************************************************
* Font class
*/
class SFont
{
public:
	//
	SFont() : fontSize(0), texId(0) {}
	//
	void Load(const std::filesystem::path& jsonPath, STexID textureId);
	//
	std::pair<SGlyph, bool> FindGlyph(wchar_t glyphCode) const;
	//
	inline bool HasGlyph(wchar_t glyphCode) const { return glyphs.find(glyphCode) != glyphs.end(); }
	//
	inline size_t GetGlyphsCount() const { return glyphs.size(); }
	//
	inline std::string GetName() const { return name; }
	//
	inline std::string GetCulture() const { return culture; }
	//
	inline std::uint32_t GetSize() const { return fontSize; }
	//
	inline STexID GetTextureId() const { return texId; }


protected:
	//
	std::map<wchar_t, SGlyph> glyphs;
	//
	std::string name;
	//
	std::string culture;
	//
	std::uint32_t fontSize;
	//
	STexID texId;

};


/***************************************************************************
* Font system
*/
class SFontSystem : public IFontSystem
{
public:
	//
	SFontSystem(const SAppContext* inContext) : context(inContext) {}


public:
	//
	virtual ~SFontSystem() {}
	//
	virtual SFontID AddFont(const std::filesystem::path& jsonPath, STexID textureId) override;
	//
	virtual std::pair<STexID, bool> GetTextureId(SFontID fontId, const std::string& culture) const override;
	//
	virtual std::pair<SGlyph, bool> FindGlyph(SFontID fontId, wchar_t glyphCode, float* outLineHeight) const override;
	// outTextSize - full string size in pixels
	virtual bool FindGlyphs(SFontID fontId, const std::wstring& text, std::vector<SGlyph>* outGlyphs, SSize2F* outTextSize) const override;
	//
	virtual class ILocalization* GetLocale() const override { return context ? context->text : nullptr; }


protected:
	//
	const SAppContext* context;
	//
	std::multimap<SFontID, SFont> fonts;
	//
	std::hash<std::string> hasher;

};
