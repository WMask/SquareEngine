/***************************************************************************
* SUtils.cpp
*/

#include "Core/SUtils.h"
#include "Core/SException.h"
#include "SConfig.h"

#include <fstream>
#include <iostream>
#include <sstream>


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
