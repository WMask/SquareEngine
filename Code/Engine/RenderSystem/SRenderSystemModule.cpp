/***************************************************************************
* SRenderSystemModule.cpp
*/

#include "RenderSystem/SRenderSystemModule.h"
#include "Application/SApplicationModule.h"


#if defined(WIN32)

#include "RenderSystem/DX11/SRenderSystemDX11.h"

std::unique_ptr<IRenderSystem> CreateRenderSystem(SRSType RenderSystemType)
{
    switch (RenderSystemType)
    {
    case SRSType::DX11:
        return std::make_unique<SRenderSystemDX11>();
    }

    return nullptr;
}

std::unique_ptr<IApplication> CreateApplication(SRSType RenderSystemType)
{
    auto app = SApplication::CreateApplication(RenderSystemType);
    auto render = CreateRenderSystem(RenderSystemType);
    app->SetRenderSystem(std::move(render));
    return app;
}

#endif
