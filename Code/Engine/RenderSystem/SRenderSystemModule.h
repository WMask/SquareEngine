/***************************************************************************
* SRenderSystemModule.h
*/

#pragma once

#include "RenderSystem/RenderSystemModule.h"
#include "RenderSystem/SRenderSystemInterface.h"
#include "Application/SApplicationInterface.h"

#include <memory>


/** Create render system */
S_RENDERSYSTEM_API
TRenderSystemPtr CreateRenderSystem(SRSType RenderSystemType);

/** Create application with render system */
S_RENDERSYSTEM_API
TApplicationPtr CreateApplication(SRSType RenderSystemType);
