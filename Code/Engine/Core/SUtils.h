/***************************************************************************
* SUtils.h
*/

#pragma once

#include "Core/CoreModule.h"
#include "Core/STypes.h"
#include <string_view>
#include <chrono>


/**
* Get engine version */
S_CORE_API
std::string_view GetEngineVersion();
/**
* Get timestamp string */
S_CORE_API
std::string GetTimeStamp(const std::chrono::system_clock::time_point& clock);
/**
* Generate resource id */
template<typename T>
inline T ResourceID(const std::string& name)
{
	static std::hash<std::string> hasher;
	return hasher(name.c_str());
}
template<typename T>
inline T ResourceID(const std::string_view& name)
{
	return ResourceID<T>(std::string(name.data()));
}


/**
* Read text file */
S_CORE_API std::string ReadTextFile(const std::filesystem::path& filePath);
/**
* Read binary file */
S_CORE_API SBytes ReadBinaryFile(const std::filesystem::path& filePath);
/**
* Write text file */
S_CORE_API void WriteTextFile(const std::filesystem::path& filePath, const std::string& text);


/**
* Read png file
* BPP - bytes per pixel (always 4)
* RowBytes - bytes per row
*/
S_CORE_API void ReadPngFile(const std::filesystem::path& filePath, std::uint32_t* outWidth, std::uint32_t* outHeight,
	std::uint32_t* outBPP, std::uint32_t* outRowBytes, void* outData = nullptr);
/**
* Read fbx file */
S_CORE_API void LoadFbxStaticMeshes(const std::filesystem::path& filePath, SGroupID groupId,
	std::forward_list<SMeshID>& meshesCacheIds, std::vector<SMesh>& outMeshes, std::vector<SMeshInstance>& outInstances);


/**
* To Utf8 string char array */
S_CORE_API std::string ToUtf8(const std::wstring& str);
/**
* From Utf8 string char array */
S_CORE_API std::wstring FromUtf8(const std::string& str);
/**
* To lower case */
S_CORE_API std::string ToLower(const char* str);
/**
* To upper case */
S_CORE_API std::string ToUpper(const char* str);
/**
* To string */
S_CORE_API std::string ToString(std::int32_t value);
/**
* To string */
S_CORE_API std::string ToString(float value);
/**
* To string */
S_CORE_API std::wstring ToStringW(std::int32_t value);
/**
* To string */
S_CORE_API std::wstring ToStringW(float value);
/**
* Localize string */
S_CORE_API std::wstring Localize(const wchar_t* fmt, ...);


/**
* Print debug string */
S_CORE_API void DebugMsg(const char* fmt, ...);
/**
* Print debug string */
S_CORE_API void DebugMsgW(const wchar_t* fmt, ...);


/**
* RAII file object */
struct S_CORE_API SFileRAII : public SUncopyable
{
	SFileRAII(const std::filesystem::path& filePath, const char* mode = "rb");
	~SFileRAII();
	operator bool() const { return file != nullptr; }
	operator FILE* () const { return file; }
	FILE* file;
};

using SPathList = std::vector<std::filesystem::path>;
