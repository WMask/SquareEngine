/***************************************************************************
* SpritesSample.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		entt::entity movingEntity;
		entt::entity rotatingEntity;

		auto onInitHandler = [&](SAppContext& context)->void
		{
			auto& registry = context.world->GetEntities();

			movingEntity = registry.create();
			auto& sprite1 = registry.emplace<SColoredSpriteComponent>(movingEntity, 0.0f, SVector3{ 200.0f, -100.0f, 0.0f }, SSize2F{ 200.0f, 200.0f });
			sprite1.SetColors(SColor3(255, 0, 0), SColor3(0, 255, 0), SColor3(0, 0, 255), SColor3(255, 255, 255));

			rotatingEntity = registry.create();
			auto& sprite2 = registry.emplace<SColoredSpriteComponent>(rotatingEntity, 0.0f, SVector3{ 400.0f, -100.0f, 0.0f }, SSize2F{ 200.0f, 200.0f });
			sprite2.SetColors(SColor3(255, 255, 255), SColor3(0, 255, 0), SColor3(255, 255, 255), SColor3(255, 255, 0));
		};

		auto onUpdateHandler = [&](float deltaSeconds, SAppContext& context)->void
		{
			auto& registry = context.world->GetEntities();
			auto view = registry.view<SColoredSpriteComponent>();

			auto& sprite1 = view.get<SColoredSpriteComponent>(movingEntity);
			sprite1.position = SVector3 {
				200.0f + sin(context.gameTime) * 32.0f,
				-100.0f + cos(context.gameTime) * 32.0f,
				0.0f
			};

			auto& sprite2 = view.get<SColoredSpriteComponent>(rotatingEntity);
			sprite2.rotation += deltaSeconds;

			DebugMsg("SpritesSample: Frame=%d, Time=%.1fs FPS=%d\n", context.gameFrame, context.gameTime, context.fps);
		};

		auto app = CreateApplication(SRSType::DX11);
		app->SetWindowSize(800, 600);
		app->SetInitHandler(onInitHandler);
		app->SetUpdateHandler(onUpdateHandler);
		app->SetFeature(SAppFeature::HighFrequencyTimer, false);
		app->SetFeature(SAppFeature::ThreadPoolEnableLogs, true);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nSpritesSample error: %s\n\n", ex.what());
	}

	return 0;
}
