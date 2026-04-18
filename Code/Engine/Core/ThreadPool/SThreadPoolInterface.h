/***************************************************************************
* SThreadPoolInterface.h
*/

#pragma once

#include "Core/STypes.h"
#include <functional>
#include <thread>


namespace SConst
{
	constexpr std::string_view DefaultTaskName = "DefaultTaskName";
	constexpr std::uint32_t MaxTasksPerThread = 512u;
	constexpr std::uint32_t MaxThreadsInPool = 32u;
	constexpr std::uint32_t DefaultTasksPerThread = 64u;
	constexpr std::uint32_t DefaultThreadsInPool = 4u;
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
	/**
	* Enable debug logging */
	virtual void EnableDebugLogs(bool bEnable) noexcept = 0;
	/**
	* Check debug logging */
	virtual bool IsDebugLogsEnabled() const noexcept = 0;

};

using TThreadPoolPtr = std::unique_ptr<IThreadPool>;
