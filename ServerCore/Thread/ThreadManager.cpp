#include "pch.h"
#include "ThreadManager.h"
#include "Utils/Utils.h"

/*----------------------------
		ThreadManager
----------------------------*/

// 초기화
void ThreadManager::Init()
{
	_activeThreadCount.store(0, std::memory_order::memory_order_relaxed);

	LOG_SYSTEM(L"ThreadManager instance initialized");
}

// 그룹별 스레드 생성 & 일감 투척
void ThreadManager::LaunchGroup(JobGroupType group, uint16 count, CallbackType jobCallback)
{
	LOCK_GUARD;

	_groupThreads[group].reserve(count);
	std::string groupName = JobGroupNames[static_cast<uint16>(group)];

	for (uint16 thread = 0; thread < count; thread++)
	{
		std::string name = groupName + "-" + std::to_string(thread);

		_groupThreads[group].emplace_back([=]()
			{
				SetThreadName(name);
				jobCallback();
			}
		);

		LOG_INFO(L"Thread Created :: " + Utils::ConvertUtf16(name));
	}
}

// 그룹별 스레드 실행 완료 대기
void ThreadManager::JoinGroup(JobGroupType group)
{
	LOCK_GUARD;

	auto it = _groupThreads.find(group);
	if (it != _groupThreads.end())
	{
		for (auto& t : it->second)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
		_groupThreads.erase(it);
	}
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

	for (auto& threads : _groupThreads)
	{
		for (auto& t : threads.second)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
	}
	_activeThreadCount.store(0, std::memory_order::memory_order_relaxed);
	_groupThreads.clear();
}

// 활성 스레드 카운트 증가
void ThreadManager::PlusActiveThreadCount()
{
	_activeThreadCount.fetch_add(1, std::memory_order::memory_order_relaxed);
}

// 활성 스레드 카운트 감소
void ThreadManager::MinusActiveThreadCount()
{
	_activeThreadCount.fetch_sub(1, std::memory_order::memory_order_relaxed);
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
