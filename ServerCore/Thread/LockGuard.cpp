#include "pch.h"
#include "LockGuard.h"

/*----------------------------
		LockGuard
----------------------------*/

LockGuard::LockGuard(Lock* lock, const char* name)
	: _lock(lock), _className(name)
{
	// Lock 요청 & 데드락 확인
	LOCK.LockRequest(lock, name);
	// 락 걸기
	_lock->ScopedLock();
	// Lock 획득
	LOCK.LockAcquired(lock);
}

LockGuard::~LockGuard()
{
	// 락 해제
	_lock->ScopedUnlock();
	LOCK.LockRelease();
}

/*----------------------------
		UniqueLockGaurd
----------------------------*/

UniqueLockGuard::UniqueLockGuard()
	: _lock(nullptr), _owns(false), _className(nullptr)
{
}

UniqueLockGuard::UniqueLockGuard(Lock* lock, const char* name)
	: _lock(lock), _owns(false), _className(name)
{
	CallLock();
}

UniqueLockGuard::~UniqueLockGuard()
{
	unlock();
}

UniqueLockGuard::UniqueLockGuard(UniqueLockGuard&& other) noexcept
	: _lock(other._lock), _owns(other._owns), _className(other._className)
{
	other._lock = nullptr;
	other._className = nullptr;
	other._owns = false;
}

UniqueLockGuard& UniqueLockGuard::operator=(UniqueLockGuard&& other) noexcept
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
		LOCK.LockRelease();
		// 소유 해제
		_owns = false;
	}
}

// lock() 호출, 데드락 확인 + 락 걸기 호출
void UniqueLockGuard::CallLock()
{
	// Lock 요청 & 데드락 확인
	LOCK.LockRequest(_lock, _className);
	// 락 걸기
	_lock->ScopedLock();
	// Lock 획득
	LOCK.LockAcquired(_lock);
	// 소유중
	_owns = true;
}
