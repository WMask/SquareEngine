/***************************************************************************
* StaticMeshSample.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputInterface.h"
#include "World/SGuiInterface.h"


static const std::string_view controlsTextKey = "controls_text";
static const std::string_view fpsTextKey = "fps_text";
static const std::string_view fpsFmtKey = "fps_fmt";

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		STextID controlsTextId = ResourceID<STextID>(controlsTextKey);
		STextID fpsTextId = ResourceID<STextID>(fpsTextKey);
		float rotation = 1.5f;
		float elevation = 350.0f;

		auto onKeys = [&](std::int32_t key, SKeyState keyState, SAppContext context)->void
		{
			if (keyState == SKeyState::Down)
			{
				switch (key)
				{
				case SKeys::Escape:
					context.app->RequestQuit();
					break;
				}
			}
		};

		auto onInitHandler = [&](SAppContext context)->void
		{
			context.input->SetKeysHandler(onKeys);

			// load resources
			auto fontTex = context.render->LoadTexture("../../Assets/Calibri_32.png");
			auto fontId = context.world->GetFonts().AddFont("../../Assets/Calibri_32.json", fontTex);
			context.text->AddCulture("../../Assets/Loc.json");
			context.render->LoadTexture("../../Assets/Barrel1.png");

			// setup widgets
			auto& registry = context.world->GetEntities();
			context.gui->MakeText(registry,
				fpsTextId, fpsTextId, fontId,
				SVector3{ 120.0f, 64.0f, 0.0f },
				SSize2F{ 128.0f, 64.0f },
				SConst::White4F, STextAlign::Begin
			);
			context.gui->MakeText(registry,
				controlsTextId, controlsTextId, fontId,
				SVector3{ 1700.0f, 64.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F, STextAlign::End
			);

			// load mesh
			const std::string_view group("Room1");
			context.render->LoadStaticMeshInstances("../../Assets/Barrel1.fbx", ResourceID<SGroupID>(group),
				[&registry](const std::filesystem::path& path, const std::vector<SMeshInstance>& instances)
			{
				for (auto& meshInstance : instances)
				{
					auto meshEntity = registry.create();
					registry.emplace<SStaticMeshComponent>(meshEntity, meshInstance.id);
					registry.emplace<STransform3DComponent>(meshEntity, meshInstance.transform);
				}
			});

			// add light
			context.world->AddDirectionalLight("DirectionalLight", SVector3{ 1.0f, -1.0f, 0.0f }, SConst::White3);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto [fmt, bFmtFound] = context.text->Get(fpsFmtKey);
			if (bFmtFound)
			{
				std::wstring locFmt = Localize(fmt.c_str(), context.fps);
				context.text->Set(fpsTextKey, locFmt);
			}

			// check input
			if (context.input->GetActiveInputDevice())
			{
				auto& keys = context.input->GetActiveInputDevice()->GetState().keys;
				if (keys[SKeys::Up])
				{
					elevation += context.deltaSeconds * 300.0f;
				}
				if (keys[SKeys::Down])
				{
					elevation -= context.deltaSeconds * 300.0f;
				}
				if (keys[SKeys::Left])
				{
					rotation += context.deltaSeconds * 3.0f;
				}
				if (keys[SKeys::Right])
				{
					rotation -= context.deltaSeconds * 3.0f;
				}

				// set camera
				context.world->GetCamera().Set(
					SVector3{ 350.0f * cos(rotation), elevation, 350.0f * sin(rotation) },
					SVector3{ 0.0f, 150.0f, 0.0f }, 60.0f
				);
			}
		};

		// set config
		auto [cfg, bCfgLoaded] = LoadConfig("../../Assets/Config.json");
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

		// run app
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
