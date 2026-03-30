/***************************************************************************
* SFontInterface.h
*/

#pragma once

#include "RenderSystem/SRenderSystemTypes.h"
#include "Core/STypes.h"

#include <filesystem>
#include <string>


/** Font glyph */
struct SGlyph
{
	// position on texture in pixels
	SVector2 pos;
	// size on texture in pixels
	SSize2F size;
};

using SFontID = std::uint32_t;

/***************************************************************************
* Font system interface
*/
class IFontSystem : public SUncopyable
{
public:
	//
	virtual ~IFontSystem() {}
	//
	virtual SFontID AddFont(const std::filesystem::path& jsonPath, STexID textureId) = 0;
	//
	virtual std::pair<STexID, bool> GetTextureId(SFontID fontId, const std::string& culture) const = 0;
	//
	virtual std::pair<SGlyph, bool> FindGlyph(SFontID fontId, wchar_t glyphCode, float* outLineHeight) const = 0;
	// outTextSize - full string size in pixels
	virtual bool FindGlyphs(SFontID fontId, const std::wstring& text, std::vector<SGlyph>& outGlyphs, SSize2F* outTextSize) const = 0;

};
