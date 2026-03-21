/***************************************************************************
* SThreadPoolInterface.h
*/

#pragma once

#include "Core/ThreadPool/Fifo4.h"
#include "Core/STypes.h"
#include <functional>
#include <thread>


namespace SConst
{
	static const std::string_view DefaultTaskName = "DefaultTaskName";
	static const std::uint32_t MaxTasksPerThread = 512u;
	static const std::uint32_t MaxThreadsInPool = 32u;
	static const std::uint32_t DefaultTasksPerThread = 64u;
	static const std::uint32_t DefaultThreadsInPool = 4u;
}


/***************************************************************************
* Thread pool interface
*/
class IThreadPool
{
public:
	using SThreadPoolTask = std::function<void()>;

public:
	/**
	* Joins all worker threads before destruction */
	virtual ~IThreadPool() {}
	/**
	* Start all worker threads */
	virtual void Start() = 0;
	/**
	* Add task to thread pool */
	virtual bool AddTask(const SThreadPoolTask& task, const std::string_view& name = SConst::DefaultTaskName) = 0;

};

using TThreadPoolPtr = std::unique_ptr<IThreadPool>;
