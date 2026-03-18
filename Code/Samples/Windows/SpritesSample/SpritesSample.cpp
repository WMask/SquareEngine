/***************************************************************************
* SpritesSample.cpp
*/

#include <windows.h>
#include "RenderSystem/SRenderSystemModule.h"
#include "World/SWorldInterface.h"
#include "Core/SUtils.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		auto OnInitHandler = [](SAppContext& context)->void
		{
			auto& entities = context.world->GetEntities();
			auto sprite = entities.create();
			entities.emplace<SColor3>(sprite, SColor3{10, 20, 30});
		};

		auto OnUpdateHandler = [](float deltaSeconds, SAppContext& context)->void
		{
			auto& entities = context.world->GetEntities();
			auto view = entities.view<SColor3>();
			for (auto entity : view)
			{
				SColor3 clr = view.get<SColor3>(entity);
				clr = SConst::OneSColor3;
			}

			DebugMsg("SpritesSample: Frame=%d, Time=%.1fs FPS=%d\n", context.gameFrame, context.gameTime, context.fps);
		};

		auto app = CreateApplication(SRSType::DX11);
		app->SetWindowSize(800, 600);
		app->SetInitHandler(OnInitHandler);
		app->SetUpdateHandler(OnUpdateHandler);
		app->SetFeature(SAppFeature::HighFrequencyTimer, false);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nSpritesSample error: %s\n\n", ex.what());
	}

	return 0;
}
