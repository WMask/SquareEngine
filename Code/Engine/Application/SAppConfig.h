/***************************************************************************
* SAppConfig.h
*/

#pragma once

#include <map>
#include <deque>
#include <filesystem>

#include "Application/ApplicationModule.h"
#include "Core/STypes.h"

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
#endif


struct SActionBinding
{
    std::string Name;
    //
    std::uint32_t Key;
    //
    std::uint32_t JoyKey;
    //
    std::uint32_t MouseBtn;
    //
    std::uint32_t AxisId;
};

struct SAppConfig
{
    SAppConfig();

    // [Application]
    std::uint32_t WinWidth;
    std::uint32_t WinHeight;
    // [Engine]
    bool bVSync;
    bool bNoDelay;
    bool bHighFrequencyTimer;
    bool bAllowResolutionChange;
    bool bEnableFXAA;
    bool bEnableHDR;
    // [Input]
    std::deque<SActionBinding> Actions;
};


/**
* Loads Application config from file */
S_APPLICATION_API std::pair<SAppConfig, bool> LoadConfig(const SPath& fileName = "config.txt",
    char delim = '\n');

/**
* Saves Application config to file */
S_APPLICATION_API void SaveConfig(const SAppConfig& config, const SPath& fileName = "config.txt");
