#include "pch.h"
#include "ThreadManager.h"

// 스레드 생성 & 일감 투척
void ThreadManager::Push(std::function<void(void)> callback)
{
	LOCK_GUARD;

	_threads.emplace_back(std::thread([=]()
		{
			callback();
			DeleteTLSData();
		}
	));
}

// 스레드 실행 완료 대기
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

// TLS 데이터 제거
void ThreadManager::DeleteTLSData()
{
	while (LHoldLock.empty() == false)
	{
		LHoldLock.pop();
	}
}
