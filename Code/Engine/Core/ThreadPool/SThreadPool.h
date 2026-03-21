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
	static const std::uint32_t MinThreads = 1u;
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
	using SCircularFIFOTaskPool = Fifo4<SThreadPoolTaskEntry>;


public:
	//
	SThreadPool(TasksPerThreadLimit maxTasksPerThread, TasksPerThreadLimit maxThreads)
		: tasksPerThread(maxTasksPerThread)
		, numThreads(maxThreads)
	{}


public: // IThreadPool interface implementation
	//
	virtual ~SThreadPool() override;
	//
	virtual void Start() override;
	//
	virtual bool AddTask(const SThreadPoolTask& task, const std::string_view& name = SConst::DefaultTaskName) override;


protected:
	//
	struct SThreadEntry
	{
		SThreadEntry(std::uint32_t tasksPerThread);
		//
		std::shared_ptr<SCircularFIFOTaskPool> tasks;
		//
		std::jthread workerThread;
	};
	//
	std::vector<SThreadEntry> pool;
	//
	std::uint32_t tasksPerThread;
	//
	std::uint32_t numThreads;

};
