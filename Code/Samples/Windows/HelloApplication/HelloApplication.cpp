/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>
#include "Core/SCoreModule.h"
#include "RenderSystem/SRenderSystemModule.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		auto onUpdateHandler = [](float deltaSeconds, SAppContext context)->void
		{
			DebugMsg("HelloApplication: Frame=%d, Time=%.1fs FPS=%d\n", context.gameFrame, context.gameTime, context.fps);
		};

		auto app = CreateApplication(SRSType::DX11);
		app->SetWindowSize(800, 600);
		app->SetUpdateHandler(onUpdateHandler);
		app->SetFeature(SAppFeature::HighFrequencyTimer, false);
		app->Init(hInstance);
		app->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nHelloApplication error: %s\n\n", ex.what());
	}

	return 0;
}
