/***************************************************************************
* SUtils.cpp
*/

#include "Core/SUtils.h"
#include "Core/SException.h"
#include "SConfig.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <png.h>

namespace SConst
{
	static const std::uint32_t MAX_PNG_SIZE = 4096u;
}


std::string_view GetEngineVersion()
{
	static const char Version[] = {
		'0' + LC_ENGINE_MAJOR_VERSION,
		'.',
		'0' + LC_ENGINE_MINOR_VERSION,
		'\0'
	};
	return Version;
}

std::string GetTimeStamp(const std::chrono::system_clock::time_point& currentTime)
{
	const std::time_t startTime = std::chrono::system_clock::to_time_t(currentTime);
	std::tm* tm = std::localtime(&startTime);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()) % 1000;
	std::uint32_t msValue = static_cast<std::uint32_t>(ms.count());
	std::ostringstream oss;
	oss << std::put_time(tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << msValue;
	return oss.str().c_str();
}

std::string ReadTextFile(const std::filesystem::path& filePath)
{
	std::string utfPath = ToUtf8(filePath.c_str());
	std::string result;

	S_TRY

	std::ifstream stream(filePath, std::ios::in | std::ios::binary);

	const auto sz = std::filesystem::file_size(filePath);
	result = std::string(sz + 1, '\0');
	stream.read(result.data(), sz);

	S_CATCH{ S_THROW_EX("ReadTextFile('", utfPath.c_str(), "')");}

	return result;
}

SBytes ReadBinaryFile(const std::filesystem::path& filePath)
{
	std::string utfPath = ToUtf8(filePath.c_str());
	SBytes result;

	S_TRY

	std::ifstream stream(filePath, std::ios::in | std::ios::binary);

	const auto sz = std::filesystem::file_size(filePath);
	result = SBytes(sz);
	stream.read(reinterpret_cast<char*>(result.data()), sz);

	S_CATCH{ S_THROW_EX("ReadBinaryFile('", utfPath.c_str(), "')"); }

	return result;
}

void WriteTextFile(const std::filesystem::path& filePath, const std::string& text)
{
	std::string utfPath = ToUtf8(filePath.c_str());

	S_TRY

	std::ofstream stream(filePath, std::ios::out);

	stream.write(text.c_str(), text.length());

	S_CATCH{ S_THROW_EX("WriteTextFile('", utfPath.c_str(), "')");}
}

void ReadPngFile(const std::filesystem::path& filePath, std::uint32_t* outWidth, std::uint32_t* outHeight,
	std::uint32_t* outBPP, std::uint32_t* outRowBytes, void* outData)
{
	struct PngRAII
	{
		PngRAII(png_structp& in_png_ptr, png_infop& in_info_ptr, png_infop& in_end_info)
			: png_ptr(in_png_ptr)
			, info_ptr(in_info_ptr)
			, end_info(in_end_info)
		{
		}
		~PngRAII() { png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); }
		png_structp& png_ptr;
		png_infop& info_ptr;
		png_infop& end_info;
	};

	S_TRY

	SFileRAII fp(filePath.string().c_str());
	if (!fp)
	{
		throw std::exception("Failed to read file");
	}

	const int sig_bytes = 8;
	png_byte png_header[sig_bytes];
	fread(png_header, 1, sig_bytes, fp);
	bool is_png = !png_sig_cmp(png_header, 0, sig_bytes);
	if (!is_png)
	{
		throw std::exception("Invalid png header");
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr)
	{
		throw std::exception("Failed to create read struct");
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, 0, 0);
		throw std::exception("Failed to create info struct");
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		throw std::exception("Failed to create end struct");
	}

	PngRAII png(png_ptr, info_ptr, end_info);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_bytes);
	png_read_info(png_ptr, info_ptr);

	std::uint32_t width = png_get_image_width(png_ptr, info_ptr);
	std::uint32_t height = png_get_image_height(png_ptr, info_ptr);
	if (!InRange(width, 0u, SConst::MAX_PNG_SIZE) ||
		!InRange(height, 0u, SConst::MAX_PNG_SIZE))
	{
		throw std::exception("Invalid image size");
	}

	auto row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	if (row_bytes == 0)
	{
		throw std::exception("Invalid row bytes");
	}

	auto color_type = png_get_color_type(png_ptr, info_ptr);
	if (color_type != PNG_COLOR_TYPE_RGB &&
		color_type != PNG_COLOR_TYPE_RGBA)
	{
		throw std::exception("Invalid color type");
	}

	auto bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	if (bit_depth != 8)
	{
		throw std::exception("Invalid bit depth");
	}

	if (outHeight) *outHeight = height;
	if (outWidth) *outWidth = width;
	if (outRowBytes) *outRowBytes = static_cast<std::uint32_t>(width * 4);
	if (outBPP) *outBPP = 4u; // Always 4 bytes per pixel
	if (outData)
	{
		std::unique_ptr<png_byte[]> tmp_row;
		if (color_type == PNG_COLOR_TYPE_RGB)
		{
			// Need temp row for BPP == 3
			tmp_row.reset(new png_byte[row_bytes]);
		}

		png_byte* rows = reinterpret_cast<png_byte*>(outData);
		for (int i = 0; i < height; i++)
		{
			if (color_type == PNG_COLOR_TYPE_RGBA)
			{
				// Read 4 bytes per pixel
				png_read_row(png_ptr, rows, 0);
			}
			else
			{
				// Read 3 bytes per pixel
				png_byte* row_pixels = tmp_row.get();
				png_read_row(png_ptr, row_pixels, 0);

				// Convert to 4 bytes
				for (int j = 0; j < width; j++)
				{
					memcpy(&rows[j * 4], &row_pixels[j * 3], 3);
					rows[j * 4 + 3] = 255;
				}
			}

			rows += width * 4;
		}
	}

	S_CATCH{ S_THROW_EX("ReadPngFile('", filePath.string().c_str(), "')"); }
}

SFileRAII::SFileRAII(const std::filesystem::path& filePath, const char* mode) : file(nullptr)
{
#ifdef _WINDOWS
	fopen_s(&file, filePath.string().c_str(), mode);
#else
	file = fopen(filePath.string().c_str(), mode);
#endif
}

SFileRAII::~SFileRAII()
{
	if (file) fclose(file);
}

#ifdef _WINDOWS

#include <windows.h>
#include <stdio.h>

std::string ToUtf8(const std::wstring& str)
{
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, NULL);
	if (requiredSize <= 0) throw std::exception("ToUtf8(): Convert error");

	std::string mbChars(requiredSize, ' ');
	int result = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &mbChars[0], (int)mbChars.length(), NULL, NULL);
	if (result != requiredSize) throw std::exception("ToUtf8(): Cannot convert");

	return mbChars;
}

std::wstring FromUtf8(const std::string& str)
{
	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
	if (requiredSize <= 0) throw std::exception("FromUtf8(): Convert error");

	std::wstring wideChars(requiredSize, ' ');
	int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wideChars[0], (int)wideChars.length());
	if (result != requiredSize) throw std::exception("FromUtf8(): Cannot convert");

	return wideChars;
}

void DebugMsg(const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	char dbg_out[4096];
	vsprintf_s(dbg_out, fmt, argp);
	va_end(argp);
	OutputDebugStringA(dbg_out);
}

void DebugMsgW(const wchar_t* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	wchar_t dbg_out[4096];
	vswprintf_s(dbg_out, fmt, argp);
	va_end(argp);
	OutputDebugStringW(dbg_out);
}

void ShowMessageModal(const char* message, const char* title)
{
	MessageBoxA(NULL, message, title, MB_OK | MB_SERVICE_NOTIFICATION);
}

#else

void DebugMsg(const char* fmt, ...) {}
void DebugMsgW(const wchar_t* fmt, ...) {}

#endif
