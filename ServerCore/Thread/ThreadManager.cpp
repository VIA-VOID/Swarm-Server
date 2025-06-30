#include "pch.h"
#include "ThreadManager.h"

/*----------------------------
		ThreadManager
----------------------------*/

// 초기화
void ThreadManager::Init()
{
	uint16 doubleThreadCount = CPU_THREAD_COUNT * 2;
	_threads.reserve(doubleThreadCount);
}

// 스레드 생성 & 일감 투척
void ThreadManager::Launch(const std::string& threadName, uint16 count, CallbackType&& jobCallback)
{
	LOCK_GUARD;

	LaunchThread(threadName, count, std::move(jobCallback));
}

// 그룹별 스레드 생성 & 일감 투척
void ThreadManager::LaunchGroup(JobGroupId groupId, uint16 count, CallbackType&& jobCallback)
{
	LOCK_GUARD;

	std::string groupName = JobGroupMgr.GetGroupName(groupId);
	LaunchThread(groupName, count, std::move(jobCallback));
}

// TLS 데이터 제거
void ThreadManager::DeleteTLSData()
{
	while (LHoldLock.empty() == false)
	{
		LHoldLock.pop();
	}
}

// 전체 스레드 실행 완료 대기
void ThreadManager::JoinAll()
{
	LOCK_GUARD;

	for (auto& thread : _threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

// 종료
void ThreadManager::Shutdown()
{
	JoinAll();
	DeleteTLSData();
}

// 스레드 이름 붙이기
// https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads?view=vs-2022&tabs=csharp
void ThreadManager::SetThreadName(const std::string& name)
{
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType;
		LPCSTR szName;
		DWORD dwThreadID;
		DWORD dwFlags;
	};
#pragma pack(pop)

	LThreadName = name;
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name.c_str();
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void ThreadManager::LaunchThread(const std::string& threadName, uint16 threadCount, CallbackType jobCallback)
{
	for (uint16 thread = 0; thread < threadCount; thread++)
	{
		std::string name = threadName + "-" + std::to_string(thread);

		_threads.emplace_back([=]()
			{
				SetThreadName(name);
				jobCallback();
			}
		);

		LOG_INFO("Thread Created :: " + name);
	}
}
