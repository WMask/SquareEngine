/***************************************************************************
* SApplicationModule.h
*/

#pragma once

#include "Application/ApplicationModule.h"
#include "Application/SLocalizationInterface.h"
#include "Application/SApplicationInterface.h"
#include "Application/SInputInterface.h"

#include <memory>


namespace SApplication
{
	/** Create application */
	S_APPLICATION_API
	TApplicationPtr CreateApplication(SRSType RenderSystemType);

	/** Create default input system */
	S_APPLICATION_API
	TInputSystemPtr CreateDefaultInputSystem(const SAppContext& context);

	/** Create localization system */
	S_APPLICATION_API
	TLocalizationPtr CreateLocalization(const SAppContext& context);

}
