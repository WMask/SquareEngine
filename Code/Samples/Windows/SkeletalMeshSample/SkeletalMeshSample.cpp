/***************************************************************************
* SkeletalMeshSample.cpp
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
	static const std::string_view CheckboxWidget = "CheckboxWidget";
	static const std::string_view BackLightWidget = "BackLightWidget";
	static const std::string_view ReflectionWidget = "ReflectionWidget";
	static const std::string_view ControlsTextKey = "controls_text";
	static const std::string_view DefaultTextKey = "default_text";
	static const std::string_view PBRTextKey = "pbr_text";
	static const std::string_view ToggleTextKey = "toggle_text";
	static const std::string_view FpsTextKey = "fps_text";
	static const std::string_view FpsFmtKey = "fps_fmt";
	static const std::string_view PolyTextKey = "poly_text";
	static const std::string_view PolyFmtKey = "poly_fmt";
	static const std::string_view BackLightTextKey = "back_light_text";
	static const std::string_view BackLightFmtKey = "back_light_fmt";
	static const std::string_view ReflectionTextKey = "reflection_text";
	static const std::string_view ReflectionFmtKey = "reflection_fmt";
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		static entt::entity pbrMeshEntity;
		static entt::entity backLightEntity;
		static entt::entity reflectionEntity;
		static const SWidgetID normalMeshButtonId = ResourceID<SWidgetID>(SConst::NormalMeshWidget);
		static const SWidgetID pbrMeshButtonId = ResourceID<SWidgetID>(SConst::PBRMeshWidget);
		static const SWidgetID normalsCheckboxId = ResourceID<SWidgetID>(SConst::NormalsWidget);
		static const SWidgetID backLightId = ResourceID<SWidgetID>(SConst::BackLightWidget);
		static const SWidgetID reflectionId = ResourceID<SWidgetID>(SConst::ReflectionWidget);
		const STextID controlsTextId = ResourceID<STextID>(SConst::ControlsTextKey);
		const STextID fpsTextId = ResourceID<STextID>(SConst::FpsTextKey);
		const STextID fpsFmtId = ResourceID<STextID>(SConst::FpsFmtKey);
		const STextID polyTextId = ResourceID<STextID>(SConst::PolyTextKey);
		const STextID polyFmtId = ResourceID<STextID>(SConst::PolyFmtKey);
		static float rotation = -2.7f;
		static float elevation = 100.0f;
		static float backLightValue = 0.3f;
		static float reflectionValue = 1.0f;

		struct GuiListener
		{
			void onMouseButtonEvent(const SMouseButtonEvent& event)
			{
				if (!registry) return;

				if (event.btn == SMouseBtn::Left && event.btnState == SKeyState::Up)
				{
					auto buttonsView = registry->view<SWidgetComponent>();
					auto& button = buttonsView.get<SWidgetComponent>(event.entity);
					if (button.bPressed)
					{
						auto meshView = registry->view<SSkeletalMeshComponent>();
						auto& pbrMesh = meshView.get<SSkeletalMeshComponent>(pbrMeshEntity);

						// show pbr mesh
						if (button.id == pbrMeshButtonId)
						{
							pbrMesh.bVisible = true;
							rotation = -2.7f;
						}
						// toggle pbr mesh normals
						else if (button.id == normalsCheckboxId)
						{
							pbrMesh.flags.bHasNormTexture = pbrMesh.flags.bHasNormTexture ? 0 : 1;
						}
					}
				}
			}
			//
			entt::registry* registry{};
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
			backLightValue = context.render->GetBackLight().r;
			reflectionValue = context.render->GetCubemapAmount(ECubemapType::Specular);

			// load resources
			context.text->AddCulture("../../Assets/Loc.json");
			context.render->LoadCubemap("../../Assets/EnvironmentSpecular.dds", ECubemapType::Specular);
			auto buttonsTex = context.render->LoadTexture("../../Assets/Buttons1.png");
			auto checkboxTex = context.render->LoadTexture("../../Assets/Checkbox1.png");
			auto fontTex = context.render->LoadTexture("../../Assets/Calibri_32.png");
			auto fontId = context.world->GetFonts().AddFont("../../Assets/Calibri_32.json", fontTex);

			// preload texture to get size
			context.render->PreloadTextures({ "../../Assets/Slider1.png" },
				[context](bool bLoaded, const std::vector<STexID>& textures)
			{
				STexID texId = textures.at(0);
				auto [texSize, bSizeFound] = context.render->GetTextureSize(texId);
				if (bSizeFound)
				{
					auto& registry = context.world->GetEntities();

					auto [backLight, bg1] = context.gui->MakeSlider(registry, texId, backLightId, backLightValue, 0.0f, 1.0f,
						SVector3{ 1720.0f, 430.0f, 0.0f }, SSize2F{ 300.0f, 40.0f }, SConvert::ToSize2F(texSize)
					);
					backLightEntity = backLight;

					auto [reflection, bg2] = context.gui->MakeSlider(registry, texId, reflectionId, reflectionValue, 0.0f, 1.0f,
						SVector3{ 1720.0f, 530.0f, 0.0f }, SSize2F{ 300.0f, 40.0f }, SConvert::ToSize2F(texSize)
					);
					reflectionEntity = reflection;
				}
			});

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
				polyTextId, polyTextId, fontId,
				SVector3{ 120.0f, 120.0f, 0.0f },
				SSize2F{ 128.0f, 64.0f },
				SConst::White4F, STextAlign::Begin
			);
			context.gui->MakeText(registry,
				controlsTextId, controlsTextId, fontId,
				SVector3{ 1700.0f, 64.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F, STextAlign::End
			);
			auto toggleTextId = ResourceID<STextID>(SConst::ToggleTextKey);
			context.gui->MakeText(registry,
				toggleTextId, toggleTextId, fontId,
				SVector3{ 1720.0f, 350.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F, STextAlign::Middle
			);
			auto backLightTextId = ResourceID<STextID>(SConst::BackLightTextKey);
			context.gui->MakeText(registry,
				backLightTextId, backLightTextId, fontId,
				SVector3{ 1720.0f, 475.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F, STextAlign::Middle
			);
			auto reflectionTextId = ResourceID<STextID>(SConst::ReflectionTextKey);
			context.gui->MakeText(registry,
				reflectionTextId, reflectionTextId, fontId,
				SVector3{ 1720.0f, 575.0f, 0.0f },
				SSize2F{ 256.0f, 64.0f },
				SConst::White4F, STextAlign::Middle
			);
			auto defaultText = ResourceID<STextID>(SConst::DefaultTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, defaultText, fontId,
				normalMeshButtonId, SVector3{ 1720.0f, 150.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }
			);
			auto pbrText = ResourceID<STextID>(SConst::PBRTextKey);
			context.gui->MakeButtonWithText(registry, buttonsTex, pbrText, fontId,
				pbrMeshButtonId, SVector3{ 1720.0f, 250.0f, 0.0f }, SSize2F{ 256.0f, 64.0f }
			);
			context.gui->MakeCheckbox(registry, checkboxTex, normalsCheckboxId, true,
				SVector3{ 1645.0f, 350.0f, 0.0f }, SSize2F{ 40.0f, 40.0f }
			);

			// load skeletal mesh
			context.render->LoadSkeletalMesh("../../Assets/Villager.fbx",
				[&](bool bLoaded, SMeshID meshId, const STransform& meshTransform, IMeshManager& manager)
			{
				if (bLoaded)
				{
					STransform transform = meshTransform;
					transform.position = SVector3{ 0.0f, -80.0f, 0.0f };

					pbrMeshEntity = registry.create();
					registry.emplace<SSkeletalMeshComponent>(pbrMeshEntity, meshId);
					registry.emplace<STransform3DComponent>(pbrMeshEntity, transform);

					manager.PreloadAnimations({ "../../Assets/Villager_Idle.fbx" }, meshId,
						[&](bool bLoaded, const std::vector<SAnimID>& anims, IMeshManager& manager)
						{
							if (!anims.empty())
							{
							}
						}
					);
				}
			});

			// add light
			SVector3 lightDir = SMath::Normalize(SVector3{ 0.8f, -0.8f, -0.2f });
			context.world->AddDirectionalLight("DirectionalLight", lightDir, SConst::White3);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto [fpsFmt, bFpsFound] = context.text->Get(SConst::FpsFmtKey);
			if (bFpsFound)
			{
				std::wstring locFmt = Localize(fpsFmt.c_str(), context.fps);
				context.text->Set(SConst::FpsTextKey, locFmt);
			}

			auto [polyFmt, bPolyFound] = context.text->Get(SConst::PolyFmtKey);
			auto [materials, bMaterialsFound] = context.render->FindMeshMaterials(pbrMeshEntity);
			if (bPolyFound && bMaterialsFound)
			{
				std::uint32_t polys = 0u;
				for (auto& meshPart : materials) polys += meshPart.numIndices / 3;
				std::wstring locFmt = Localize(polyFmt.c_str(), polys);
				context.text->Set(SConst::PolyTextKey, locFmt);
			}

			auto& registry = context.world->GetEntities();
			auto backLightComponent = registry.try_get<SSliderComponent>(backLightEntity);
			if (backLightComponent && backLightValue != backLightComponent->value)
			{
				backLightValue = backLightComponent->value;
				context.render->SetBackLight(SColor3F{ backLightValue, backLightValue, backLightValue });

				auto [backLightFmt, bBackFound] = context.text->Get(SConst::BackLightFmtKey);
				if (bBackFound)
				{
					auto value = std::lroundf(backLightValue * 100.0f);
					std::wstring locFmt = Localize(backLightFmt.c_str(), value);
					context.text->Set(SConst::BackLightTextKey, locFmt);
				}
			}

			auto reflectionComponent = registry.try_get<SSliderComponent>(reflectionEntity);
			if (reflectionComponent && reflectionValue != reflectionComponent->value)
			{
				reflectionValue = reflectionComponent->value;
				context.render->SetCubemapAmount(reflectionValue, ECubemapType::Specular);

				auto [refFmt, bRefFound] = context.text->Get(SConst::ReflectionFmtKey);
				if (bRefFound)
				{
					auto value = std::lroundf(reflectionValue * 100.0f);
					std::wstring locFmt = Localize(refFmt.c_str(), value);
					context.text->Set(SConst::ReflectionTextKey, locFmt);
				}
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
					rotation += context.deltaSeconds * 1.5f;
				}
				if (keys[SKeys::Right])
				{
					rotation -= context.deltaSeconds * 1.5f;
				}

				// set camera
				context.world->GetCamera().Set(
					SVector3{ 200.0f * cos(rotation), elevation, 200.0f * sin(rotation) },
					SVector3{ 0.0f, 0.0f, 0.0f }, 60.0f
				);
			}
		};

		// set features
		auto app = CreateApplication(SRSType::DX11);
		app->SetFeature(SAppFeature::HighFrequencyTimer, false);
		app->SetFeature(SAppFeature::NoDelay, false);
		app->SetFeature(SAppFeature::VSync, true);
		app->SetFeature(SAppFeature::EnableFXAA, false);
		app->SetWindowSize(1920, 1080);

		// run app
		app->SetInitHandler(onInitHandler);
		app->SetUpdateHandler(onUpdateHandler);
		app->SetFeature(SAppFeature::ThreadPoolDebugTrace, false);
		app->SetFeature(SAppFeature::RenderSystemDebugTrace, false);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nStaticMeshSample error: %s\n\n", ex.what());
	}

	return 0;
}
