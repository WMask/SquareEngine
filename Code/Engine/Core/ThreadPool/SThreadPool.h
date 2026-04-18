/***************************************************************************
* SThreadPool.h
*/

#pragma once

#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "Core/ThreadPool/Fifo4.h"
#include "Core/STypes.h"
#include <functional>
#include <thread>


namespace SConst
{
	constexpr std::uint32_t MinThreads = 1u;
}


/***************************************************************************
* Thread pool
*/
class SThreadPool : public IThreadPool
{
public:
	struct SThreadPoolTaskEntry
	{
		std::function<void()> task;
		std::string_view name;
	};
	using TasksPerThreadLimit = TRange<std::uint32_t, SConst::MinThreads, SConst::MaxTasksPerThread>;
	using ThreadsLimit = TRange<std::uint32_t, SConst::MinThreads, SConst::MaxThreadsInPool>;
	using TCircularFIFOTaskPool = Fifo4<SThreadPoolTaskEntry>;


public:
	//
	SThreadPool(TasksPerThreadLimit maxTasksPerThread, TasksPerThreadLimit maxThreads)
		: tasksPerThread(maxTasksPerThread)
		, numThreads(maxThreads)
		, bEnableDebugLogs(false)
	{}


public: // IThreadPool interface implementation
	//
	virtual ~SThreadPool() override;
	//
	virtual void Start() override;
	//
	virtual bool AddTask(const SThreadPoolTask& task, const std::string_view& name = SConst::DefaultTaskName) override;
	//
	virtual void EnableDebugLogs(bool bEnable) noexcept override { bEnableDebugLogs = bEnable; }
	//
	virtual bool IsDebugLogsEnabled() const noexcept override { return bEnableDebugLogs; }


protected:
	//
	struct SThreadEntry
	{
		SThreadEntry(std::uint32_t tasksPerThread, std::uint32_t threadId, bool bEnableDebugLogs);
		//
		std::shared_ptr<TCircularFIFOTaskPool> tasks;
		//
		std::jthread workerThread;
	};
	//
	std::vector<SThreadEntry> pool;
	//
	std::uint32_t tasksPerThread;
	//
	std::uint32_t numThreads;
	//
	bool bEnableDebugLogs;

};
