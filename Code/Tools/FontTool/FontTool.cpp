/***************************************************************************
* FontTool.cpp
*/

#include <iostream>
#include <vector>
#include <codecvt>
#include "Core/SUtils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"


std::string MakeFontName(const std::filesystem::path& path);
int FindMaxVertOffset(const stbtt_fontinfo& info, unsigned int bitmapSize, unsigned int lineHeight, unsigned int firstChar, unsigned int charCount);

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cout << "\nRequired arguments:\n";
        std::cout << "1. Path to font file (char*)\n";
        std::cout << "2. Output image height in pixels (optional) (default: 256)\n";
        std::cout << "3. Line height in pixels         (optional) (default: 32)\n";
        std::cout << "4. Offset to first character     (optional) (default: 32)\n";
        std::cout << "5. Characters count              (optional) (default: 96)\n";
        return 1;
    }

    // in pixels
    unsigned int bitmapSize = 256;

    if (argc > 2)
    {
        bitmapSize = atoi(argv[2]);
    }

    // in pixels
    unsigned int lineHeight = 32;
    if (argc > 3)
    {
        lineHeight = atoi(argv[3]);
    }

    unsigned int firstChar = 32;
    if (argc > 4)
    {
        firstChar = atoi(argv[4]);
    }

    unsigned int charCount = 96;
    if (argc > 5)
    {
        charCount = atoi(argv[5]);
    }

    std::filesystem::path fontPath(argv[1]);
    auto fontBuffer = ReadBinaryFile(fontPath);

    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, fontBuffer.data(), 0))
    {
        std::cout << "\nFailed to init font.\n";
        return 3;
    }

    std::vector<unsigned char> bitmap;
    bitmap.resize(bitmapSize * bitmapSize, sizeof(unsigned char));

    float scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(lineHeight));

    int x = 0, y = FindMaxVertOffset(info, bitmapSize, lineHeight, firstChar, charCount);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    std::cout << "First char code: " << firstChar << "\n";
    std::cout << "Initial Y offset: " << y << " px\n";

    ascent = static_cast<int>(roundf(ascent * scale));
    descent = static_cast<int>(roundf(descent * scale));

    struct LETTER
    {
        float x;
        float y;
        float w;
        float h;
    };

    std::vector<LETTER> letters;
    letters.reserve(charCount);

    for (unsigned int i = firstChar; i < firstChar + charCount; ++i)
    {
        int posx;
        int offsetx;
        stbtt_GetCodepointHMetrics(&info, i, &posx, &offsetx);

        float dx = posx * scale;
        if (x + dx > static_cast<float>(bitmapSize))
        {
            // advance y
            y += lineHeight;
            x = 0;

            if (y + lineHeight > bitmapSize)
            {
                std::cout << "\nInvalid output image size.\n";
                return 4;
            }
        }

        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &x1, &y1, &x2, &y2);

        int localy = ascent + y1;
        int byteOffset = static_cast<int>(x + roundf(offsetx * scale) + ((localy + y) * bitmapSize));

        stbtt_MakeCodepointBitmap(&info, bitmap.data() + byteOffset, x2 - x1, y2 - y1, bitmapSize, scale, scale, i);

        // advance x
        x += static_cast<int>(roundf(posx * scale));

        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, i, i + 1);
        x += static_cast<int>(roundf(kern * scale));

        letters.emplace_back(LETTER{
            static_cast<float>(x),
            static_cast<float>(y),
            posx * scale,
            static_cast<float>(y2 - y1)
        });
    }

    // write letters atlas in png
    stbi_write_png("1.png", bitmapSize, bitmapSize, 1, bitmap.data(), bitmapSize);

    // write letters data to json
    SFileRAII jsonFile("1.json", "w");
    if (!jsonFile)
    {
        std::cout << "\nFailed to write json file.\n";
        return 5;
    }

    std::string fontName = MakeFontName(fontPath);
    std::cout << "Font name: " << fontName << "\n";

    fprintf(jsonFile, "{\n\t\"displayName\": \"%s\",\n\t\"fontSize\": %d,\n\t\"firstCharCode\": %d,\n\t\"glyphs\": [\n", fontName.c_str(), lineHeight, firstChar);

    charCount = static_cast<int>(letters.size());

    for (size_t i = 0; i < charCount; i++)
    {
        wchar_t cc[3] = { static_cast<wchar_t>(i + firstChar), '\0', '\0' };
        if (cc[0] == '"') { cc[0] = cc[1] = '\''; }
        if (cc[0] == '\\') { cc[0] = cc[1] = '\\'; }
        std::string utf8_string = ToUtf8(cc);

        fprintf(jsonFile, "\t\t{\n\t\t\t\"glyph\": \"%s\",\n\t\t\t\"pos\": {\"x\": %.3f, \"y\": %.3f},\n\t\t\t\"size\": {\"x\": %.3f, \"y\": %.3f}\n\t\t}",
            utf8_string.c_str(), letters[i].x, letters[i].y, letters[i].w, letters[i].h);
        fprintf(jsonFile, (i == (charCount - 1)) ? "\n" : ",\n");
    }

    fprintf(jsonFile, "\t]\n}\n");

    return 0;
}

std::string MakeFontName(const std::filesystem::path& fontPath)
{
    std::string fontName(fontPath.stem().string());

    if (InRange(fontName[0], 'a', 'z'))
    {
        fontName[0] = std::toupper(fontName[0]);
    }

    return fontName;
}

int FindMaxVertOffset(const stbtt_fontinfo& info, unsigned int bitmapSize, unsigned int lineHeight, unsigned int firstChar, unsigned int charCount)
{
    float scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(lineHeight));
    int x = 0, y = lineHeight / 2;
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent = static_cast<int>(roundf(ascent * scale));
    descent = static_cast<int>(roundf(descent * scale));

    int MaxVertOffset = 0;

    for (unsigned int i = firstChar; i < firstChar + charCount; ++i)
    {
        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &x1, &y1, &x2, &y2);

        int localy = ascent + y1;
        if (localy < 0 && (-localy) > MaxVertOffset)
        {
            MaxVertOffset = (-localy);
        }
    }

    return MaxVertOffset;
}
