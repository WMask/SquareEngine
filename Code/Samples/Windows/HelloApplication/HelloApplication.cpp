/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputInterface.h"
#include "World/SGuiInterface.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		const std::string_view textKey = "demo_text";
		entt::entity texturedEntity;
		entt::entity buttonEntity;
		entt::entity textEntity;
		STextID textId;
		SFontID fontId;

		auto onKeys = [](std::int32_t key, SKeyState keyState, SAppContext context)->void
		{
			switch (key)
			{
			case SKeys::Escape:
				context.app->RequestQuit();
				break;
			}
		};

		auto onInitHandler = [&](SAppContext context)->void
		{
			context.input->SetKeysHandler(onKeys);

			// fonts and localization
			auto texId1 = context.render->LoadTexture("../../Assets/Calibri_32.png");
			fontId = context.world->GetFonts().AddFont("../../Assets/Calibri_32.json", texId1);
			auto texId2 = context.render->LoadTexture("../../Assets/Calibri_32_ru.png");
			context.world->GetFonts().AddFont("../../Assets/Calibri_32_ru.json", texId2);
			context.text->AddCulture("../../Assets/Loc.json");
			context.text->AddCulture("../../Assets/Loc_ru.json");
			context.text->SetCulture("en");

			auto& registry = context.world->GetEntities();

			// textured entity
			texturedEntity = registry.create();
			auto& sprite1 = registry.emplace<SColoredSpriteComponent>(
				texturedEntity, true, 0.0f,
				SVector3{ 300.0f, 300.0f, 0.0f },
				SSize2F{ 256.0f, 256.0f }
			);
			sprite1.SetWhiteColors();
			auto& texUV1 = registry.emplace<SSpriteUVComponent>(texturedEntity);
			texUV1.SetDefaultUV();
			auto texId3 = context.render->LoadTexture("../../Assets/Tree1.png");
			registry.emplace<STexturedComponent>(texturedEntity, texId3);

			// button entity
			buttonEntity = registry.create();
			auto& sprite2 = registry.emplace<SColoredSpriteComponent>(
				buttonEntity, true, 0.0f,
				SVector3{ 300.0f, 500.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f }
			);
			sprite2.SetWhiteColors();
			auto& texUV2 = registry.emplace<SSpriteUVComponent>(buttonEntity);
			texUV2.SetDefaultUV();
			for (auto& uv : texUV2.uvs) uv.y /= 2.0f;
			auto texId4 = context.render->LoadTexture("../../Assets/Buttons1.png");
			registry.emplace<STexturedComponent>(buttonEntity, texId4);

			// text entity
			textEntity = registry.create();
			auto textId = context.text->MakeId(textKey);
			registry.emplace<STextComponent>(
				textEntity,
				SVector3{ 230.0f, 500.0f, 0.1f },
				SSize2F{ 256.0f, 256.0f },
				SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f },
				textId, fontId
			);
			registry.emplace<SWidgetComponent>(textEntity);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
		};

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
