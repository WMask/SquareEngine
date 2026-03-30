/***************************************************************************
* SApplicationModule.cpp
*/

#include "Application/SApplicationModule.h"


#if defined(WIN32)

#include "Application/Windows/SWindowsApplication.h"

namespace SApplication
{
    TApplicationPtr CreateApplication(SRSType RenderSystemType)
    {
        return std::make_unique<SWindowsApplication>();
    }
}

#endif
