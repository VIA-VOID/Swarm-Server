#include "pch.h"
#include "Lock.h"

/*----------------------------
		Lock
----------------------------*/

void Lock::ScopedLock()
{
	_mutex.lock();
}

void Lock::ScopedUnlock()
{
	_mutex.unlock();
}

/*----------------------------
		DeadlockDetector
----------------------------*/

// 초기화
void DeadlockDetector::Init()
{
#ifdef _DEBUG
	IsCheck.store(true, std::memory_order::memory_order_relaxed);
	LOG_SYSTEM(L"DeadlockDetector instance initialized");
#endif;
}

// lock 요청 & 데드락 체크
void DeadlockDetector::LockRequest(Lock* lock, const char* name)
{
	if (IsCheck.load())
	{
		ThreadId threadId = std::this_thread::get_id();
		LockAddress reqLockAddress = reinterpret_cast<LockAddress>(lock);
		// 현재 소유하고 있는 lock이 있다면 사이클 체크
		if (LHoldLock.empty() == false)
		{
			LockAddress hold = LHoldLock.top();

			std::lock_guard<std::mutex> guard(_mutex);
			{
				_lockGraph[hold].insert(reqLockAddress);
				_lockLog.insert_or_assign(reqLockAddress, std::make_pair(threadId, name));

				// 데드락 탐지
				std::vector<LockAddress> visited;
				if (CycleCheck(reqLockAddress, reqLockAddress, visited))
				{
					visited.push_back(reqLockAddress);
					CrashDeadLock(visited);
				}
			}
		}
	}
}

// lock 획득
void DeadlockDetector::LockAcquired(Lock* lock)
{
	if (IsCheck.load())
	{
		LockAddress addr = reinterpret_cast<LockAddress>(lock);
		LHoldLock.push(addr);
	}
}

// 소유하고 있는 lock 제거
void DeadlockDetector::LockRelease()
{
	if (IsCheck.load())
	{
		if (LHoldLock.empty() == false)
		{
			LHoldLock.pop();
		}
	}
}

// 종료
void DeadlockDetector::Shutdown()
{
#ifdef _DEBUG
	IsCheck.store(false, std::memory_order::memory_order_relaxed);
#endif;
}

// 교차상태(순환) 있는지 확인
// 데드락 탐지
bool DeadlockDetector::CycleCheck(LockAddress current, LockAddress cycleLock, std::vector<LockAddress>& visited)
{
	// 방문(체크)한 적이 있으다면 리턴
	if (std::find(visited.begin(), visited.end(), cycleLock) != visited.end())
	{
		return false;
	}
	// 방문
	visited.push_back(cycleLock);

	// lock을 건적이 있는지 체크
	const auto it = _lockGraph.find(cycleLock);
	if (it == _lockGraph.end())
	{
		return false;
	}
	// 있다면 동일한 주소의 락이 있는지 사이클 체크
	for (LockAddress lockAddr : it->second)
	{
		// 현재 시도하려는 lock과 일치하는 주소가 있다면 데드락
		if (current == lockAddr)
		{
			return true;
		}
		// dfs 반복
		if (CycleCheck(current, lockAddr, visited))
		{
			return true;
		}
	}
	return false;
}

// 데드락 발생시 CRASH
void DeadlockDetector::CrashDeadLock(std::vector<LockAddress>& visited)
{
	std::wstringstream ws;
	ws << L"====================================================\n";
	ws << L"[DEADLOCK_DETECTED]\n";
	ws << L"Cycle Path (Thread ID) : (Class Name)\n";

	for (LockAddress addr : visited)
	{
		auto log = _lockLog.find(addr);
		ThreadId threadId = log->second.first;
		const char* className = log->second.second;

		ws << L" → Thread " << threadId << L" : " << className << L"\n";
	}

	ws << L"====================================================\n";
	std::wcout << ws.str();
	CRASH("DEADLOCK_DETECTED !!!!");
}
