/***************************************************************************
* SUtils.cpp
*/

#include "Core/SUtils.h"
#include "SConfig.h"


std::string_view SGetEngineVersion()
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

#ifdef _WINDOWS

#include <windows.h>
#include <stdio.h>

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
