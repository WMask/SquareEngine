/***************************************************************************
* SRenderSystemModule.h
*/

#pragma once

#include "RenderSystem/SRenderSystem.h"
#include "RenderSystem/SRenderSystemInterface.h"

#include <memory>


/** Create render system */
S_RENDERSYSTEM_API
std::unique_ptr<IRenderSystem> CreateRenderSystem(SRSType RenderSystemType);
