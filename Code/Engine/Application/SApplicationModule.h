/***************************************************************************
* SApplicationModule.h
*/

#pragma once

#include "Application/SApplication.h"
#include "Application/SApplicationInterface.h"

#include <memory>


namespace SApplication
{
	/** Create application */
	S_APPLICATION_API
	std::unique_ptr<IApplication> CreateApplication(SRSType RenderSystemType);
}
