/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputSystem.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		auto onKeys = [](std::int32_t key, SKeyState keyState, SAppContext context)->void
		{
			switch (key)
			{
			case SKeys::Escape:
				if (context.app) context.app->RequestQuit();
				break;
			}
		};

		auto onInitHandler = [&](SAppContext context)->void
		{
			if (context.input)
			{
				context.input->SetKeysHandler(onKeys);
			}
		};

		auto onUpdateHandler = [](float deltaSeconds, SAppContext context)->void
		{
			DebugMsg("HelloApplication: Frame=%d, Time=%.1fs FPS=%d\n", context.gameFrame, context.gameTime, context.fps);
		};

		auto [cfg, bCfgLoaded] = LoadConfig("../../Code/Samples/Assets/config.json");
		auto app = CreateApplication(SRSType::DX11);
		if (bCfgLoaded)
		{
			app->SetConfig(cfg);
			app->SetWindowSize(cfg.WinWidth, cfg.WinHeight);
			app->SetFeature(SAppFeature::HighFrequencyTimer, cfg.bHighFrequencyTimer);
			app->SetFeature(SAppFeature::AllowFullscreen, cfg.bAllowFullscreen);
			app->SetFeature(SAppFeature::NoDelay, cfg.bNoDelay);
			app->SetFeature(SAppFeature::VSync, cfg.bVSync);
		}
		else
		{
			app->SetFeature(SAppFeature::HighFrequencyTimer, false);
			app->SetFeature(SAppFeature::VSync, true);
			app->SetWindowSize(800, 600);
		}

		app->SetInitHandler(onInitHandler);
		app->SetUpdateHandler(onUpdateHandler);
		app->SetFeature(SAppFeature::ThreadPoolDebugTrace, true);
		app->SetFeature(SAppFeature::RenderSystemDebugTrace, true);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nHelloApplication error: %s\n\n", ex.what());
	}

	return 0;
}
