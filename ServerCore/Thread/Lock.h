#pragma once

/*--------------------------------------------------------
					Lock

- Lock은 클래스마다 생성
- Lock 실행/해제는 LockGuard에서 수행
--------------------------------------------------------*/
class Lock
{
public:
	void ScopedLock();
	void ScopedUnlock();

private:
	std::mutex _mutex;
};

/*--------------------------------------------------------
				LockDebugManager

- 데드락 확인은 DebugMode에서만 사용
--------------------------------------------------------*/
class LockDebugManger : public Singleton<LockDebugManger>
{
public:
	// 데드락 확인
	void CheckDeadLock(Lock& lock);
	// 락 해제시 소유권 제거
	void UnLock();

private:
	// 교차상태(순환) 있는지 확인
	void CheckCycle(std::thread::id threadId, std::uintptr_t lockAddr);

private:
	std::mutex _managerMutex;
	// lock 요청한 스레드
	std::unordered_map<std::thread::id, std::unordered_set<std::uintptr_t>> _reqLock;
	// lock을 획득한 스레드
	std::unordered_map<std::uintptr_t, std::thread::id> _owner;
	// 사이클 경로 추적용도
	std::vector<std::thread::id> _path;
};
