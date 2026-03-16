/***************************************************************************
* SCore.h
*/

#pragma once


#if defined(_MSC_VER)

#ifndef SCore_EXPORTS
#  define S_CORE_API __declspec (dllimport)
#else
#  define S_CORE_API __declspec (dllexport)
#endif

#endif
