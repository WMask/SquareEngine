/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputInterface.h"
#include "World/SGuiInterface.h"


namespace SConst
{
	static const std::string_view ToggleButtonWidget = "ToggleButton";
	static const std::string_view ApplyButtonWidget = "ApplyButton";
	static const std::string_view FpsTextWidget = "FpsText";
	static const std::string_view ToggleTextKey = "demo_text";
	static const std::string_view ApplyTextKey = "apply_text";
	static const std::string_view FpsTextKey = "fps_text";
	static const std::string_view FpsFmtKey = "fps_fmt";
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		static const SWidgetID toggleButtonId = ResourceID<SWidgetID>(SConst::ToggleButtonWidget);
		static const SWidgetID applyButtonId = ResourceID<SWidgetID>(SConst::ApplyButtonWidget);
		static const SWidgetID fpsTextId = ResourceID<SWidgetID>(SConst::FpsTextWidget);
		SColor4F targetTint = SConst::White4F;
		SColor4F tint = SConst::White4F;
		entt::entity treeEntity;
		float lerp = 1.0f;
		bool bUseGreen = true;

		static auto startSpriteAnim = [&]()
		{
			targetTint = bUseGreen ? SColor4F{ 0.0f, 0.6f, 0.0f, 1.0f } : SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f };
			bUseGreen = !bUseGreen;
			lerp = 0.0f;
		};

		struct GuiListener
		{
			void onMouseButtonEvent(const SMouseButtonEvent& event)
			{
				if (!registry || !locale) return;

				if (event.btn == SMouseBtn::Left && event.btnState == SKeyState::Up)
				{
					auto buttonsView = registry->view<SWidgetComponent, SButtonComponent>();
					auto& button = buttonsView.get<SWidgetComponent>(event.entity);
					if (button.bPressed)
					{
						// toggle sprite tint
						if (button.id == toggleButtonId)
						{
							startSpriteAnim();
						}
						// toggle localization
						else if (button.id == applyButtonId)
						{
							locale->SetCulture((locale->GetCulture() == "en") ? "ru" : "en");
						}
					}
				}
			}
			//
			entt::registry* registry{};
			ILocalization* locale{};
		};
		GuiListener listener;

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

			// setup widgets
			auto& registry = context.world->GetEntities();
			context.gui->onMouseEvent.sink<SMouseButtonEvent>().connect<&GuiListener::onMouseButtonEvent>(listener);
			listener.registry = &registry;
			listener.locale = context.text;

			// load resources
			auto treeTex = context.render->LoadTexture("../../Assets/Tree1.png");
			auto buttonsTex = context.render->LoadTexture("../../Assets/Buttons1.png");
			auto fontTex1 = context.render->LoadTexture("../../Assets/Calibri_32.png");
			auto fontTex2 = context.render->LoadTexture("../../Assets/Calibri_32_ru.png");
			auto fontId = context.world->GetFonts().AddFont("../../Assets/Calibri_32.json", fontTex1);
			context.world->GetFonts().AddFont("../../Assets/Calibri_32_ru.json", fontTex2);
			context.text->AddCulture("../../Assets/Loc.json");
			context.text->AddCulture("../../Assets/Loc_ru.json");
			context.text->SetCulture("en");

			// tree entity
			treeEntity = context.gui->MakeTexturedSprite(registry, treeTex,
				SVector3{ 700.0f, 300.0f, 0.0f }, SSize2F{ 256.0f, 256.0f },
				SConst::White4F
			);

			// toggle button
			auto toggleText = ResourceID<STextID>(SConst::ToggleTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, toggleText, fontId, toggleButtonId,
				SVector3{ 700.0f, 500.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);

			// apply button
			auto applyText = ResourceID<STextID>(SConst::ApplyTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, applyText, fontId, applyButtonId,
				SVector3{ 300.0f, 500.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);

			// fps text
			auto fpsText = ResourceID<STextID>(SConst::FpsTextKey);
			context.gui->MakeText(registry, fpsTextId, fpsText, fontId,
				SVector3{ 300.0f, 300.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto [fmt, bFmtFound] = context.text->Get(SConst::FpsFmtKey);
			if (bFmtFound)
			{
				std::wstring locFmt = Localize(fmt.c_str(), context.fps);
				context.text->Set(SConst::FpsTextKey, locFmt);
			}

			if (lerp < 1.0f)
			{
				auto& registry = context.world->GetEntities();
				auto spriteView = registry.view<SColoredComponent>();
				auto& sprite = spriteView.get<SColoredComponent>(treeEntity);

				tint = SMath::LerpColor4(tint, targetTint, lerp);
				lerp += deltaSeconds / 4.0f;
				sprite.SetColors(tint);
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
