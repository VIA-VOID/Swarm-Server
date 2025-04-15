#pragma once
/*-------------------------------------------------------
				LockGuard

- Custom LockGuard
- 표준 lock_Guard 기능, 데드락 탐지 추가
--------------------------------------------------------*/
class LockGuard
{
public:
	LockGuard(Lock* lock, const char* name);
	~LockGuard();

	// 복사방지
	LockGuard(const Lock& lock) = delete;
	LockGuard& operator=(const Lock& lock) = delete;

private:
	Lock* _lock;
	// 데드락 디버깅용 - 클래스명
	const char* _className;
};

/*-------------------------------------------------------
				UniqueLockGaurd

- Custom UniqueLockGuard
- 표준 unique_lock + lock_Guard 기능, 데드락 탐지 추가
- debug 모드에선 customLock 사용을 위한 condition_variable_any 사용
--------------------------------------------------------*/
class UniqueLockGuard
{
public:
	UniqueLockGuard();
	// 생성자 & Lock 소유
	UniqueLockGuard(Lock* lock, const char* name);
	~UniqueLockGuard();

	// 이동 생성자
	// 표준 unique_lock에선 이동만 허용, 복사 금지
	UniqueLockGuard(UniqueLockGuard&& other) noexcept;
	UniqueLockGuard& operator=(UniqueLockGuard&& other) noexcept;

	// 복사방지
	UniqueLockGuard(const Lock& lock) = delete;
	UniqueLockGuard& operator=(const Lock& lock) = delete;

	// --------------------------------------------------
	// condition_variable_any 사용을 위한 BasicLockable
	// https://learn.microsoft.com/en-us/cpp/standard-library/condition-variable?view=msvc-170
	// - at Remarks
	// https://en.cppreference.com/w/cpp/thread/condition_variable_any
	// - operate on any lock that meets the BasicLockable requirements.
	void lock();
	void unlock();
	// --------------------------------------------------

private:
	// lock() 호출, 데드락 확인 + 락 걸기 호출
	void CallLock();

private:
	Lock* _lock;
	// unlock -> unlock 호출 방지
	bool _owns;
	// 데드락 디버깅용 - 클래스명
	const char* _className;
};