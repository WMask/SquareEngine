/***************************************************************************
* SConfig.h
*/

#pragma once


/***************************************************************************
* Engine version
*/
#define LC_ENGINE_MAJOR_VERSION  0
#define LC_ENGINE_MINOR_VERSION  1


/***************************************************************************
* Math libs
*/
#define USING_EIGEN   1
#define USING_GLMATH  0


/***************************************************************************
* Config verification
*/
#if \
	(USING_GLMATH && USING_EIGEN) || \
	(!USING_GLMATH && !USING_EIGEN)
# error "Square engine: Wrong Math lib configuration"
#endif
