/***************************************************************************
* SAppConfig.cpp
*/

#include "Application/SAppConfig.h"
#include "Core/SUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;


SAppConfig::SAppConfig()
{
	WinWidth = 800;
	WinHeight = 600;
	bVSync = true;
	bNoDelay = false;
	bHighFrequencyTimer = false;
	bAllowResolutionChange = false;
	bEnableFXAA = false;
	bEnableHDR = false;
}

std::pair<SAppConfig, bool> LoadConfig(const std::filesystem::path& fileName, char delim)
{
	auto fullConfigText = ReadTextFile(fileName);
	if (fullConfigText.length() < 7) return { SAppConfig(), false};

	json cfg = json::parse(fullConfigText);

	SAppConfig result{};
	result.WinWidth				  = cfg["Application"]["WinWidth"].get<unsigned int>();
	result.WinHeight			  = cfg["Application"]["WinHeight"].get<unsigned int>();
	result.bVSync				  = cfg["Engine"]["bVSync"].get<bool>();
	result.bNoDelay				  = cfg["Engine"]["bNoDelay"].get<bool>();
	result.bHighFrequencyTimer	  = cfg["Engine"]["bHighFrequencyTimer"].get<bool>();
	result.bAllowResolutionChange = cfg["Engine"]["bAllowResolutionChange"].get<bool>();
	result.bEnableFXAA			  = cfg["Engine"]["bEnableFXAA"].get<bool>();
	result.bEnableHDR			  = cfg["Engine"]["bEnableHDR"].get<bool>();

	for (auto action : cfg["Input"])
	{
		SActionBinding binding{};
		binding.Name     = action["Name"].get<std::string>();
		binding.Key      = action["Key"].get<int>();
		binding.JoyKey   = action["JoyKey"].get<int>();
		binding.MouseBtn = action["MouseBtn"].get<int>();
		binding.AxisId   = action["AxisId"].get<int>();
		result.Actions.push_back(binding);
	}

	return { result, true };
}

void SaveConfig(const SAppConfig& config, const std::filesystem::path& fileName)
{
	json cfg = {
		{"Application", {
			{"WinWidth",			config.WinWidth},
			{"WinHeight",			config.WinHeight}
		}},
		{"Engine", {
			{"bVSync",				   config.bVSync},
			{"bNoDelay",			   config.bNoDelay},
			{"bHighFrequencyTimer",	   config.bHighFrequencyTimer},
			{"bAllowResolutionChange", config.bAllowResolutionChange},
			{"bEnableFXAA",			   config.bEnableFXAA},
			{"bEnableHDR",			   config.bEnableHDR}
		}},
		{"Input", {}}
	};

	for (auto& action : config.Actions)
	{
		json binding = {
			{"Name", action.Name},
			{"Key", action.Key},
			{"JoyKey", action.JoyKey},
			{"MouseBtn", action.MouseBtn},
			{"AxisId", action.AxisId},
		};
		cfg["Input"].push_back(binding);
	}

	WriteTextFile(fileName, cfg.dump(4) + "\n");
}
