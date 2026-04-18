/***************************************************************************
* SpritesSample.cpp
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
		entt::entity movingEntity;
		entt::entity rotatingEntity;
		entt::entity texturedEntity;
		entt::entity animatedEntity;

		auto onKeys = [](std::int32_t key, SKeyState keyState, SAppContext context)->void
		{
			switch (key)
			{
			case SKeys::Escape:
				if (keyState == SKeyState::Down) context.app->RequestQuit();
				break;
			}
		};

		auto onInitHandler = [&](SAppContext context)->void
		{
			context.input->SetKeysHandler(onKeys);

			auto& registry = context.world->GetEntities();
			auto treeTex = context.textures->LoadTexture("../../Assets/Tree1.png");
			auto boomTex = context.textures->LoadTexture("../../Assets/Boom1.png");

			movingEntity = context.gui->MakeColoredSprite(registry,
				SVector3{ 300.0f, 300.0f, 0.0f }, SSize2F{ 256.0f, 256.0f },
				SColor3{ 255, 0, 0 }, SColor3{ 0, 255, 0 },
				SColor3{ 0, 0, 255 }, SColor3{ 255, 255, 255 }
			);

			rotatingEntity = context.gui->MakeColoredSprite(registry,
				SVector3{ 750.0f, 300.0f, 0.0f }, SSize2F{ 256.0f, 256.0f },
				SColor3{ 0, 255, 0 }, SColor3{ 255, 255, 255 },
				SColor3{ 255, 255, 0 }, SColor3{ 255, 255, 255 }
			);

			texturedEntity = context.gui->MakeTexturedSprite(registry, treeTex,
				SVector3{ 300.0f, 700.0f, 0.0f }, SSize2F{ 256.0f, 256.0f },
				SConst::White4F
			);

			animatedEntity = context.gui->MakeAnimatedSprite(registry, boomTex,
				SVector3{ 750.0f, 680.0f, 0.0f }, SSize2F{ 256.0f, 256.0f },
				SConst::White4F, 0, 50, 25, SSize2{ 100, 100 }, context.gameTime
			);
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext context)->void
		{
			auto& registry = context.world->GetEntities();
			auto view = registry.view<SSpriteComponent>();

			// move
			auto& sprite1 = view.get<SSpriteComponent>(movingEntity);
			sprite1.position = SVector3 {
				300.0f + sin(context.gameTime) * 32.0f,
				300.0f + cos(context.gameTime) * 32.0f,
				0.0f
			};

			// rotate
			auto& sprite2 = view.get<SSpriteComponent>(rotatingEntity);
			sprite2.rotation += deltaSeconds;

			// check input
			if (context.input->GetActiveInputDevice())
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
