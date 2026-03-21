/***************************************************************************
* SUtils.h
*/

#pragma once

#include "Core/CoreModule.h"
#include <string_view>
#include <chrono>


/**
* Get engine version */
S_CORE_API
std::string_view SGetEngineVersion();

/**
* Get timestamp string */
S_CORE_API
std::string GetTimeStamp(const std::chrono::system_clock::time_point& clock);

/**
* Print debug string */
S_CORE_API void DebugMsg(const char* fmt, ...);
/**
* Print debug string */
S_CORE_API void DebugMsgW(const wchar_t* fmt, ...);
