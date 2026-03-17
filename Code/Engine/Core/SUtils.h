/***************************************************************************
* SUtils.h
*/

#pragma once

#include "Core/SCore.h"
#include <string_view>


/**
* Get engine version */
S_CORE_API
std::string_view SGetEngineVersion();

/**
* Print debug string */
S_CORE_API void DebugMsg(const char* fmt, ...);
/**
* Print debug string */
S_CORE_API void DebugMsgW(const wchar_t* fmt, ...);
