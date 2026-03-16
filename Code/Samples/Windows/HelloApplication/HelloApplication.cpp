/***************************************************************************
* HelloApplication.cpp
*/

#include <windows.h>

#include "Core/SUtils.h"
#include "Core/SException.h"
#include "Application/SApplicationModule.h"
#include "RenderSystem/SRenderSystemModule.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	try
	{
		auto Render = CreateRenderSystem(SRSType::DX11);
		auto App = CreateApplication(SRSType::DX11);
		App->SetRenderSystem(std::move(Render));
		App->SetWindowSize(800, 600);
		App->Init(hInstance);
		App->Run();
	}
	catch (const std::exception& ex)
	{
		DebugMsg("\nHelloApplication error:\n%s\n\n", ex.what());
	}

	return 0;
}
