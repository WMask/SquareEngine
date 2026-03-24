/***************************************************************************
* SCoreModule.h
*/

#pragma once

#include "Core/STypes.h"
#include "Core/SMath.h"
#include "Core/SUtils.h"
#include "Core/ThreadPool/SThreadPoolInterface.h"


/** Create thread pool */
S_CORE_API
TThreadPoolPtr CreateThreadPool(std::uint32_t maxTasksPerThread = SConst::DefaultTasksPerThread, std::uint32_t maxThreads = SConst::DefaultThreadsInPool);
