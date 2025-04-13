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
	LockGuard
----------------------------*/

LockGuard::LockGuard(Lock* lock)
	: _lock(lock)
{
#if _DEBUG
	// 데드락 확인
	LOCK_M.CheckDeadLock(lock);
#endif

	// 락 걸기
	_lock->ScopedLock();
}

LockGuard::~LockGuard()
{
	// 락 해제
	_lock->ScopedUnlock();

#if _DEBUG
	LOCK_M.UnLock();
#endif
}

/*----------------------------
	LockDebugManger
----------------------------*/

// 교차상태(순환) 있는지 확인
void LockDebugManger::CheckCycle(std::thread::id threadId, std::uintptr_t lockAddr)
{
	// 락을 점유한 주인이 없다면 리턴
	auto owner = _owner.find(lockAddr);
	if (owner == _owner.end())
	{
		return;
	}

	// 락 점유한 주인의 스레드 ID
	auto ownerId = owner->second;

	// 이미 _path에 ownerId가 포함되어 있다면 사이클이 존재함 (데드락 감지)
	if (std::find(_path.begin(), _path.end(), ownerId) != _path.end())
	{
		// TODO: 로그찍기
		CRASH("DEADLOCK_DETECTED!!!");
	}

	// 방문 기록
	_path.push_back(threadId);

	// ownerId가 요청한 다른 락을 확인하고 재귀적으로 탐색
	auto findReqId = _reqLock.find(ownerId);
	if (findReqId != _reqLock.end())
	{
		auto& reqSet = findReqId->second;
		for (auto& addr : reqSet)
		{
			CheckCycle(ownerId, addr);
		}
	}

	// 사이클 미발생, 방문 기록 제거
	_path.pop_back();
}

// 데드락 확인
void LockDebugManger::CheckDeadLock(Lock* lock)
{
	std::lock_guard<std::mutex> guard(_managerMutex);
	// 스레드 ID
	const std::thread::id threadId = std::this_thread::get_id();
	// debug 편의를 위해 주소값으로 변환 후 map에 추가
	const std::uintptr_t lockAddr = reinterpret_cast<std::uintptr_t>(lock);

	// 만일 현재 스레드가 가지고 있는 lock 확인
	if (LThreadLock.empty() == false)
	{
		// 가지고 있는 lock 비교, 새로운 lock이라면 사이클 체크
		const std::uintptr_t getTopLock = LThreadLock.top();
		if (getTopLock != lockAddr)
		{
			// 락 요청한 스레드 목록에 추가
			auto& reqSet = _reqLock[threadId];
			reqSet.insert(lockAddr);

			// 사이클 체크
			CheckCycle(threadId, lockAddr);
		}
	}

	// lock 소유권 획득
	_owner.insert({ lockAddr, threadId });
	LThreadLock.push(lockAddr);
}

// 락 해제시 소유권 제거
void LockDebugManger::UnLock()
{
	auto lockAddr = LThreadLock.top();
	_owner.erase(lockAddr);

	const std::thread::id threadId = std::this_thread::get_id();
	auto req = _reqLock.find(threadId);
	if (req != _reqLock.end())
	{
		auto reqSet = req->second;
		if (reqSet.find(lockAddr) != reqSet.end())
		{
			reqSet.erase(lockAddr);
		}
	}

	LThreadLock.pop();
}
