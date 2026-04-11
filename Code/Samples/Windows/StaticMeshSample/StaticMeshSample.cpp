/***************************************************************************
* StaticMeshSample.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SInputInterface.h"
#include "World/SGuiInterface.h"


namespace SConst
{
	static const std::string_view NormalMeshWidget = "NormalMeshWidget";
	static const std::string_view PBRMeshWidget = "PBRMeshWidget";
	static const std::string_view NormalsWidget = "NormalsWidget";
	static const std::string_view ControlsTextKey = "controls_text";
	static const std::string_view DefaultTextKey = "default_text";
	static const std::string_view PBRTextKey = "pbr_text";
	static const std::string_view ToggleTextKey = "toggle_text";
	static const std::string_view FpsTextKey = "fps_text";
	static const std::string_view FpsFmtKey = "fps_fmt";
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		entt::entity pbrMeshEntity;
		entt::entity normalMeshEntity;
		const STextID controlsTextId = ResourceID<STextID>(SConst::ControlsTextKey);
		const STextID fpsTextId = ResourceID<STextID>(SConst::FpsTextKey);
		const STextID fpsFmtId = ResourceID<STextID>(SConst::FpsFmtKey);
		float rotation = 0.73f;
		float elevation = 10.0f;

		struct GuiListener
		{
			GuiListener(entt::entity& inPBRMeshEntity, entt::entity& inNormalMeshEntity)
				: normalMeshEntity(inNormalMeshEntity)
				, pbrMeshEntity(inPBRMeshEntity)
			{
				normalMeshButtonId = ResourceID<SWidgetID>(SConst::NormalMeshWidget);
				pbrMeshButtonId = ResourceID<SWidgetID>(SConst::PBRMeshWidget);
				normalsButtonId = ResourceID<SWidgetID>(SConst::NormalsWidget);
			}
			//
			void onMouseButtonEvent(const SMouseButtonEvent& event)
			{
				if (!registry) return;

				if (event.btn == SMouseBtn::Left && event.btnState == SKeyState::Up)
				{
					auto buttonsView = registry->view<SWidgetComponent>();
					auto& button = buttonsView.get<SWidgetComponent>(event.entity);
					if (button.bPressed)
					{
						auto meshView = registry->view<SStaticMeshComponent>();
						auto& pbrMesh = meshView.get<SStaticMeshComponent>(pbrMeshEntity);
						auto& normalMesh = meshView.get<SStaticMeshComponent>(normalMeshEntity);

						// show normal mesh
						if (button.id == normalMeshButtonId)
						{
							pbrMesh.bVisible = false;
							normalMesh.bVisible = true;
						}
						// show pbr mesh
						else if (button.id == pbrMeshButtonId)
						{
							pbrMesh.bVisible = true;
							normalMesh.bVisible = false;
						}
						// toggle pbr mesh normals
						else if (button.id == normalsButtonId)
						{
							pbrMesh.flags.bHasNormTexture = pbrMesh.flags.bHasNormTexture ? 0 : 1;
						}
					}
				}
			}
			//
			entt::registry* registry{};
			entt::entity& pbrMeshEntity;
			entt::entity& normalMeshEntity;
			SWidgetID normalMeshButtonId;
			SWidgetID pbrMeshButtonId;
			SWidgetID normalsButtonId;
		};
		GuiListener listener(pbrMeshEntity, normalMeshEntity);

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
			context.render->LoadCubemap("../../Assets/EnvironmentDiffuse.dds", ECubemapType::Diffuse);
			context.render->LoadCubemap("../../Assets/EnvironmentSpecular.dds", ECubemapType::Specular);
			auto buttonsTex = context.render->LoadTexture("../../Assets/Buttons1.png");
			auto fontTex = context.render->LoadTexture("../../Assets/Calibri_32.png");
			auto fontId = context.world->GetFonts().AddFont("../../Assets/Calibri_32.json", fontTex);
			context.text->AddCulture("../../Assets/Loc.json");

			// setup widgets
			auto& registry = context.world->GetEntities();
			context.gui->onMouseEvent.sink<SMouseButtonEvent>().connect<&GuiListener::onMouseButtonEvent>(listener);
			listener.registry = &registry;

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
			auto defaultText = ResourceID<STextID>(SConst::DefaultTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, defaultText, fontId, listener.normalMeshButtonId,
				SVector3{ 1720.0f, 150.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);
			auto pbrText = ResourceID<STextID>(SConst::PBRTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, pbrText, fontId, listener.pbrMeshButtonId,
				SVector3{ 1720.0f, 250.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);
			auto toggleText = ResourceID<STextID>(SConst::ToggleTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, toggleText, fontId, listener.normalsButtonId,
				SVector3{ 1720.0f, 350.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }, SConst::White4F
			);

			// load meshes
			const std::string_view group("Room1");
			context.render->LoadStaticMeshInstances("../../Assets/Axe1.fbx", ResourceID<SGroupID>(group),
				[&](const std::filesystem::path& path, const std::vector<SMeshInstance>& instances)
			{
				if (!instances.empty())
				{
					auto& meshInstance = instances[0];
					auto transform = meshInstance.transform;
					pbrMeshEntity = registry.create();
					registry.emplace<SStaticMeshComponent>(pbrMeshEntity, meshInstance.id);
					registry.emplace<STransform3DComponent>(pbrMeshEntity, transform);
				}
			});
			context.render->LoadStaticMeshInstances("../../Assets/Barrel1.fbx", ResourceID<SGroupID>(group),
				[&](const std::filesystem::path& path, const std::vector<SMeshInstance>& instances)
			{
				if (!instances.empty())
				{
					auto& meshInstance = instances[0];
					auto transform = meshInstance.transform;
					transform.scale = transform.scale * 0.8f;
					transform.pos.y = transform.pos.y + 32.0f;

					normalMeshEntity = registry.create();
					registry.emplace<SStaticMeshComponent>(normalMeshEntity, meshInstance.id, false);
					registry.emplace<STransform3DComponent>(normalMeshEntity, transform);
				}
			});

			// add light
			SVector3 lightDir = SMath::Normalize(SVector3{ 0.0f, -0.8f, 0.5f });
			context.world->AddDirectionalLight("DirectionalLight", lightDir, SConst::White3);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto [fmt, bFmtFound] = context.text->Get(SConst::FpsFmtKey);
			if (bFmtFound)
			{
				std::wstring locFmt = Localize(fmt.c_str(), context.fps);
				context.text->Set(SConst::FpsTextKey, locFmt);
			}

			// check input
			if (context.input->GetActiveInputDevice())
			{
				auto& keys = context.input->GetActiveInputDevice()->GetState().keys;
				if (keys[SKeys::Up])
				{
					elevation += context.deltaSeconds * 100.0f;
				}
				if (keys[SKeys::Down])
				{
					elevation -= context.deltaSeconds * 100.0f;
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
					SVector3{ 70.0f * cos(rotation), elevation, 70.0f * sin(rotation) },
					SVector3{ 0.0f, 0.0f, 0.0f }, 60.0f
				);
			}
		};

		// set config
		auto [cfg, bCfgLoaded] = LoadConfig("../../Assets/Config.json");
		auto app = CreateApplication(SRSType::DX11);
		if (!bCfgLoaded)
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
			app->SetFeature(SAppFeature::HighFrequencyTimer, cfg.bHighFrequencyTimer);
			app->SetFeature(SAppFeature::AllowFullscreen, cfg.bAllowFullscreen);
			app->SetFeature(SAppFeature::NoDelay, cfg.bNoDelay);
			app->SetFeature(SAppFeature::VSync, cfg.bVSync);
			app->SetWindowSize(1920, 1080);
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
