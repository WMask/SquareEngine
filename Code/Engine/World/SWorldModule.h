/***************************************************************************
* SWorldModule.h
*/

#pragma once

#include "World/WorldModule.h"
#include "World/SGuiInterface.h"
#include "World/SWorldInterface.h"
#include "Application/SApplicationTypes.h"


/** Create world */
S_WORLD_API
TWorldPtr CreateWorld(const SAppContext& context);

/** Create GUI system */
S_WORLD_API
TGuiSystemPtr CreateGuiSystem(const SAppContext& context);
