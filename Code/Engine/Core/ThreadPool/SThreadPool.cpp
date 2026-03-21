/***************************************************************************
* SThreadPool.cpp
*/

#include "Core/ThreadPool/SThreadPool.h"
#include "Core/SCoreModule.h"
#include <chrono>


void TaskThreadBody(std::stop_token stopToken, std::shared_ptr<SThreadPool::SCircularFIFOTaskPool> pool, std::uint32_t threadId, bool bEnableDebugLogs)
{
	using namespace std::chrono_literals;

	if (bEnableDebugLogs)
	{
		DebugMsg("[%s] SThreadPoolWorker[#%d] started\n", GetTimeStamp(std::chrono::system_clock::now()).c_str(), threadId);
	}

	while (true)
	{
		if (stopToken.stop_requested())
		{
			break;
		}

		SThreadPool::SThreadPoolTaskEntry entry;
		if (pool->pop(entry))
		{
			auto startTime = std::chrono::system_clock::now();
			if (bEnableDebugLogs)
			{
				DebugMsg("[%s] SThreadPoolWorker[#%d] begin task: '%s'\n", GetTimeStamp(startTime).c_str(), threadId, entry.name.data());
			}

			entry.task();

			if (bEnableDebugLogs)
			{
				auto endTime = std::chrono::system_clock::now();
				std::chrono::duration<float> timeSeconds = endTime - startTime;
				DebugMsg("[%s] SThreadPoolWorker[#%d] end task: '%s', duration: %.3f sec\n", GetTimeStamp(std::chrono::system_clock::now()).c_str(), threadId, entry.name.data(), timeSeconds.count());
			}
		}
		else
		{
			std::this_thread::sleep_for(1ms);
		}
	}

	if (bEnableDebugLogs)
	{
		DebugMsg("[%s] SThreadPoolWorker[#%d] finished\n", GetTimeStamp(std::chrono::system_clock::now()).c_str(), threadId);
	}
}

SThreadPool::SThreadEntry::SThreadEntry(std::uint32_t tasksPerThread, std::uint32_t threadId, bool bEnableDebugLogs)
	: tasks{ std::make_shared<SCircularFIFOTaskPool>(tasksPerThread) }
	, workerThread(TaskThreadBody, tasks, threadId, bEnableDebugLogs)
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
		for (std::uint32_t i = 0; i < numThreads; i++)
		{
			pool.emplace_back(tasksPerThread, i + 1, bEnableDebugLogs);
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
