/***************************************************************************
* SThreadPool.cpp
*/

#include "Core/ThreadPool/SThreadPool.h"
#include "Core/SCoreModule.h"
#include <chrono>


void TaskThreadBody(std::stop_token stopToken, std::shared_ptr<SThreadPool::SCircularFIFOTaskPool> pool)
{
	using namespace std::chrono_literals;

	while (true)
	{
		if (stopToken.stop_requested())
		{
			break;
		}

		SThreadPool::SThreadPoolTaskEntry entry;
		if (pool->pop(entry))
		{
			entry.task();
		}
		else
		{
			std::this_thread::sleep_for(1ms);
		}
	}
}

SThreadPool::SThreadEntry::SThreadEntry(std::uint32_t tasksPerThread)
	: tasks{ std::make_shared<SCircularFIFOTaskPool>(tasksPerThread) }
	, workerThread(TaskThreadBody, tasks)
{
}

SThreadPool::~SThreadPool()
{
	for (auto& entry : pool)
	{
		if (entry.workerThread.joinable())
		{
			entry.workerThread.request_stop();
		}
	}
}

void SThreadPool::Start()
{
	if (pool.size() == 0)
	{
		for (auto i = 0; i < numThreads; i++)
		{
			pool.emplace_back(tasksPerThread);
		}
	}
}

bool SThreadPool::AddTask(const SThreadPoolTask& task, const std::string_view& name)
{
	if (pool.size() != 0)
	{
		std::int32_t randomThreadId = rand() % pool.size();

		if (pool[randomThreadId].tasks->push({ task, name }))
		{
			return true;
		}
	}

	return false;
}

TThreadPoolPtr CreateThreadPool(std::uint32_t maxTasksPerThread, std::uint32_t maxThreads)
{
	return std::make_unique<SThreadPool>(maxTasksPerThread, maxThreads);
}
