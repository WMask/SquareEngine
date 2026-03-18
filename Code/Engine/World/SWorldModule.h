/***************************************************************************
* SWorldModule.h
*/

#pragma once

#include "World/WorldModule.h"
#include "World/SWorldInterface.h"
#include "Application/SApplicationTypes.h"


/** Create world */
S_WORLD_API
TWorldPtr CreateWorld(const SAppContext& context);
