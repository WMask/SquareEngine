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
	bAllowFullscreen = false;
	bHighFrequencyTimer = false;
	bNoDelay = false;
}

std::pair<SAppConfig, bool> LoadConfig(const std::filesystem::path& fileName, char delim)
{
	auto fullConfigText = ReadTextFile(fileName);
	if (fullConfigText.length() < 7) return { SAppConfig(), false};

	json cfg = json::parse(fullConfigText);

	SAppConfig result{};
	result.WinWidth				= cfg["Application"]["WinWidth"].get<unsigned int>();
	result.WinHeight			= cfg["Application"]["WinHeight"].get<unsigned int>();
	result.bVSync				= cfg["Engine"]["bVSync"].get<bool>();
	result.bAllowFullscreen		= cfg["Engine"]["bAllowFullscreen"].get<bool>();
	result.bHighFrequencyTimer	= cfg["Engine"]["bHighFrequencyTimer"].get<bool>();
	result.bNoDelay				= cfg["Engine"]["bNoDelay"].get<bool>();

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
			{"bVSync",				config.bVSync},
			{"bAllowFullscreen",	config.bAllowFullscreen},
			{"bHighFrequencyTimer",	config.bHighFrequencyTimer},
			{"bNoDelay",			config.bNoDelay}
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
