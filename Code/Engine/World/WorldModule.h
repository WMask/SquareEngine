/***************************************************************************
* WorldModule.h
*/

#pragma once


#if defined(_MSC_VER)

#ifndef SWorld_EXPORTS
# define S_WORLD_API __declspec (dllimport)
#else
# define S_WORLD_API __declspec (dllexport)
#endif

#else

# define S_WORLD_API 

#endif
