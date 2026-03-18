/***************************************************************************
* RenderSystemModule.h
*/

#pragma once


#if defined(_MSC_VER)

#ifndef SRenderSystem_EXPORTS
# define S_RENDERSYSTEM_API __declspec (dllimport)
#else
# define S_RENDERSYSTEM_API __declspec (dllexport)
#endif

#else

# define S_RENDERSYSTEM_API 

#endif
