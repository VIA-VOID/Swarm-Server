#include "pch.h"
#include "LockGuard.h"

/*----------------------------
		LockGuard
----------------------------*/

LockGuard::LockGuard(Lock& lock)
	: _lock(&lock)
{
	// 데드락 확인
	LOCK_M.CheckDeadLock(lock);
	// 락 걸기
	_lock->ScopedLock();
}

LockGuard::~LockGuard()
{
	// 락 해제
	_lock->ScopedUnlock();
	LOCK_M.UnLock();
}

/*----------------------------
		UniqueLockGaurd
----------------------------*/

UniqueLockGuard::UniqueLockGuard()
	: _lock(nullptr), _owns(false)
{
}

UniqueLockGuard::UniqueLockGuard(Lock& lock)
	: _lock(&lock), _owns(false)
{
	CallLock();
}

UniqueLockGuard::~UniqueLockGuard()
{
	unlock();
}

UniqueLockGuard::UniqueLockGuard(UniqueLockGuard&& other)
	: _lock(other._lock), _owns(other._owns)
{
	other._lock = nullptr;
	other._owns = false;
}

UniqueLockGuard& UniqueLockGuard::operator=(UniqueLockGuard&& other)
{
	if (this != &other)
	{
		unlock();
		// 이동
		_lock = other._lock;
		_owns = other._owns;

		other._lock = nullptr;
		other._owns = false;
	}
	return *this;
}

// 데드락 확인 + 락 걸기 호출
void UniqueLockGuard::lock()
{
	ASSERT_CRASH(_lock != nullptr);
	if (_owns == false)
	{
		CallLock();
	}
}

// 락 해제 호출
void UniqueLockGuard::unlock()
{
	ASSERT_CRASH(_lock != nullptr);
	if (_owns)
	{
		// 락 해제
		_lock->ScopedUnlock();
		LOCK_M.UnLock();
		// 소유 해제
		_owns = false;
	}
}

// lock() 호출, 데드락 확인 + 락 걸기 호출
void UniqueLockGuard::CallLock()
{
	// 데드락 확인
	LOCK_M.CheckDeadLock(*_lock);
	// 락 걸기
	_lock->ScopedLock();
	// 소유중
	_owns = true;
}
