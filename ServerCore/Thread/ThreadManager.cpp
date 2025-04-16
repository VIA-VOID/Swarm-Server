#include "pch.h"
#include "ThreadManager.h"

// 스레드 생성 & 일감 투척
void ThreadManager::Push(std::function<void(void)> callback)
{
	LOCK_GUARD;

	_threads.push_back(std::thread([=]() { callback(); }));
}

void ThreadManager::Join()
{
	for (std::thread& thread : _threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
	_threads.clear();
}
