/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputInterface.h"
#include "World/SGuiInterface.h"


static const std::string_view toggleTextWidget = "toggleText";
static const std::string_view toggleButtonWidget = "toggleButton";
static const std::string_view applyButtonWidget = "applyButton";
static const std::string_view applyTextWidget = "applyText";
static const std::string_view fpsTextWidget = "fpsText";
static const std::string_view toggleTextKey = "demo_text";
static const std::string_view applyTextKey = "apply_text";
static const std::string_view fpsTextKey = "fps_text";
static const std::string_view fpsFmtKey = "fps_fmt";

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		struct GuiListener
		{
			GuiListener()
			{
				toggleTextId = ResourceID<SWidgetID>(toggleTextWidget);
				toggleButtonId = ResourceID<SWidgetID>(toggleButtonWidget);
				applyButtonId = ResourceID<SWidgetID>(applyButtonWidget);
				applyTextId = ResourceID<SWidgetID>(applyTextWidget);
				fpsTextId = ResourceID<SWidgetID>(fpsTextWidget);
			}
			//
			void onMouseButtonEvent(const SMouseButtonEvent& event)
			{
				if (!registry || !locale) return;

				if (event.btn == SMouseBtn::Left)
				{
					auto widgetView = registry->view<SWidgetComponent>();
					auto& widget = widgetView.get<SWidgetComponent>(event.entity);

					if (widget.id == toggleButtonId || widget.id == applyButtonId)
					{
						// update button uv
						auto uvView = registry->view<SSpriteUVComponent>();
						auto& uv = uvView.get<SSpriteUVComponent>(event.entity);
						(event.btnState == SKeyState::Up) ? uv.SetTopHalfUV() : uv.SetBottomHalfUV();

						if (event.btnState == SKeyState::Up && widget.bPressed)
						{
							// toggle sprite tint
							if (widget.id == toggleButtonId)
							{
								targetTint = bUseGreen ? SColor4F{ 0.0f, 0.6f, 0.0f, 1.0f } : SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f };
								bUseGreen = !bUseGreen;
								lerp = 0.0f;
							}
							// toggle localization
							else if (widget.id == applyButtonId)
							{
								locale->SetCulture((locale->GetCulture() == "en") ? "ru" : "en");
							}
						}
					}
					else if (widget.id == toggleTextId || widget.id == applyTextId)
					{
						// update button text
						auto spriteView = registry->view<SSpriteComponent>();
						auto& sprite = spriteView.get<SSpriteComponent>(event.entity);
						sprite.position.y = (event.btnState == SKeyState::Up) ? 500.0f : 501.0f;
					}
				}
			}
			//
			void onMouseLeaveEvent(const SMouseLeaveEvent& event)
			{
				if (!registry) return;

				auto widgetView = registry->view<SWidgetComponent>();
				auto& widget = widgetView.get<SWidgetComponent>(event.entity);

				if (widget.id == toggleButtonId || widget.id == applyButtonId)
				{
					// update button uv
					auto uvView = registry->view<SSpriteUVComponent>();
					auto& uv = uvView.get<SSpriteUVComponent>(event.entity);
					uv.SetTopHalfUV();
				}
				else if (widget.id == toggleTextId || widget.id == applyTextId)
				{
					// update button text
					auto spriteView = registry->view<SSpriteComponent>();
					auto& text = spriteView.get<SSpriteComponent>(event.entity);
					text.position.y = 500.0f;
				}
			}
			//
			entt::registry* registry{};
			ILocalization* locale{};
			SWidgetID toggleButtonId;
			SWidgetID toggleTextId;
			SWidgetID applyButtonId;
			SWidgetID applyTextId;
			SWidgetID fpsTextId;
			SColor4F tint = SConst::White4F;
			SColor4F targetTint = SConst::White4F;
			float lerp = 1.0f;
			bool bUseGreen = true;
		};

		entt::entity treeEntity;
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
			context.gui->onMouseEvent.sink<SMouseLeaveEvent>().connect<&GuiListener::onMouseLeaveEvent>(listener);
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
			auto toggleText = ResourceID<STextID>(toggleTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, toggleText, fontId,
				listener.toggleButtonId, listener.toggleTextId,
				SVector3{ 700.0f, 500.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F
			);

			// apply button
			auto applyText = ResourceID<STextID>(applyTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, applyText, fontId,
				listener.applyButtonId, listener.applyTextId,
				SVector3{ 300.0f, 500.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F
			);

			// fps text
			auto fpsText = ResourceID<STextID>(fpsTextKey);
			context.gui->MakeText(registry,
				listener.fpsTextId, fpsText, fontId,
				SVector3{ 300.0f, 300.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F
			);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto [fmt, bFmtFound] = context.text->Get(fpsFmtKey);
			if (bFmtFound)
			{
				std::wstring locFmt = Localize(fmt.c_str(), context.fps);
				context.text->Set(fpsTextKey, locFmt);
			}

			if (listener.lerp < 1.0f)
			{
				auto& registry = context.world->GetEntities();
				auto spriteView = registry.view<SColoredComponent>();
				auto& sprite = spriteView.get<SColoredComponent>(treeEntity);

				listener.tint = SMath::LerpColor4(listener.tint, listener.targetTint, listener.lerp);
				listener.lerp += deltaSeconds / 4.0f;
				sprite.SetColors(listener.tint);
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
