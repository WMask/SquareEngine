/***************************************************************************
* SpritesSample.cpp
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
		entt::entity movingEntity;
		entt::entity rotatingEntity;
		entt::entity texturedEntity;
		entt::entity animatedEntity;

		auto onKeys = [](std::int32_t key, SKeyState keyState, SAppContext context)->void
		{
			switch (key)
			{
			case SKeys::Escape:
				if (context.app && keyState == SKeyState::Down) context.app->RequestQuit();
				break;
			}
		};

		auto onInitHandler = [&](SAppContext context)->void
		{
			if (context.input)
			{
				context.input->SetKeysHandler(onKeys);
			}

			auto& registry = context.world->GetEntities();

			// moving entity
			movingEntity = registry.create();
			auto& sprite1 = registry.emplace<SColoredSpriteComponent>(
				movingEntity, 0.0f,
				SVector3{ 300.0f, 300.0f, 0.0f },
				SSize2F{ 256.0f, 256.0f }
			);
			sprite1.SetColors(SColor3(255, 0, 0), SColor3(0, 255, 0),
				SColor3(0, 0, 255), SColor3(255, 255, 255));

			// rotating entity
			rotatingEntity = registry.create();
			auto& sprite2 = registry.emplace<SColoredSpriteComponent>(
				rotatingEntity, 0.0f,
				SVector3{ 750.0f, 300.0f, 0.0f },
				SSize2F{ 256.0f, 256.0f }
			);
			sprite2.SetColors(SColor3(0, 255, 0), SColor3(255, 255, 255),
				SColor3(255, 255, 0), SColor3(255, 255, 255));

			// textured entity
			texturedEntity = registry.create();
			auto& sprite3 = registry.emplace<SColoredSpriteComponent>(
				texturedEntity, 0.0f,
				SVector3{ 300.0f, 700.0f, 0.0f },
				SSize2F{ 256.0f, 256.0f }
			);
			sprite3.SetWhiteColors();
			auto& texUV3 = registry.emplace<SSpriteUVComponent>(texturedEntity);
			texUV3.SetDefaultUV();
			auto texId1 = context.render->LoadTexture("../../Code/Samples/Assets/T_Tree1.png");
			registry.emplace<STexturedComponent>(texturedEntity, texId1);

			// animated entity
			animatedEntity = registry.create();
			auto& sprite4 = registry.emplace<SColoredSpriteComponent>(
				animatedEntity, 0.0f,
				SVector3{ 750.0f, 680.0f, 0.0f },
				SSize2F{ 256.0f, 256.0f }
			);
			sprite4.SetWhiteColors();
			auto texId2 = context.render->LoadTexture("../../Code/Samples/Assets/T_Boom1.png");
			registry.emplace<STexturedComponent>(animatedEntity, texId2);
			auto& anim4 = registry.emplace<SSpriteFrameAnimComponent>(animatedEntity);
			anim4.SetAnim(0, 50, 25, SSize2{ 100, 100 }, 0.0f);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto& registry = context.world->GetEntities();
			auto view = registry.view<SColoredSpriteComponent>();

			// move
			auto& sprite1 = view.get<SColoredSpriteComponent>(movingEntity);
			sprite1.position = SVector3 {
				300.0f + sin(context.gameTime) * 32.0f,
				300.0f + cos(context.gameTime) * 32.0f,
				0.0f
			};

			// rotate
			auto& sprite2 = view.get<SColoredSpriteComponent>(rotatingEntity);
			sprite2.rotation += deltaSeconds;

			// check input
			if (context.input &&
				context.input->GetActiveInputDevice())
			{
				auto& keys = context.input->GetActiveInputDevice()->GetState().keys;
				if (keys[SKeys::Up])
				{
					sprite2.position.y -= context.deltaSeconds * 350.0f;
				}

				if (keys[SKeys::Down])
				{
					sprite2.position.y += context.deltaSeconds * 350.0f;
				}
			}
		};

		auto app = CreateApplication(SRSType::DX11);
		app->SetWindowSize(800, 600);
		app->SetInitHandler(onInitHandler);
		app->SetUpdateHandler(onUpdateHandler);
		app->SetFeature(SAppFeature::VSync, true);
		app->SetFeature(SAppFeature::HighFrequencyTimer, false);
		app->SetFeature(SAppFeature::ThreadPoolDebugTrace, true);
		app->SetFeature(SAppFeature::RenderSystemDebugTrace, true);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nSpritesSample error: %s\n\n", ex.what());
	}

	return 0;
}
