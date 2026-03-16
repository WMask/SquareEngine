/***************************************************************************
* SApplication.h
*/

#pragma once

#if defined(_MSC_VER)

#ifndef SApplication_EXPORTS
# define S_APPLICATION_API __declspec (dllimport)
#else
# define S_APPLICATION_API __declspec (dllexport)
#endif

#else

# define S_APPLICATION_API 

#endif
