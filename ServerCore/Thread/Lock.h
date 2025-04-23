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
					DeadlockDetector

- LockDebugManger의 개선된 버전
- Thread Local Storage 에서 요청한 lock, 소유한 lock 관리
- 데드락 탐지를 위해 lock을 잡아 데드락이 걸리는 기이한? 현상 제거
--------------------------------------------------------*/
class DeadlockDetector : public Singleton<DeadlockDetector>
{
	using LockAddress = uint64;

public:
	// 초기화
	void Init() override;
	// lock 요청 & 데드락 체크
	void LockRequest(Lock* lock, const char* name);
	// lock 획득
	void LockAcquired(Lock* lock);
	// 소유하고 있는 lock 제거
	void LockRelease();
	// 종료
	void Shutdown() override;

private:
	// 교차상태(순환) 있는지 확인
	bool CycleCheck(LockAddress start, LockAddress current, std::vector<LockAddress>& visited);
	// 데드락 발생시 CRASH
	void CrashDeadLock(std::vector<LockAddress>& visited);

private:
	std::mutex _mutex;
	// 데드락 체크 여부
	// - debug에서만 활성화
	std::atomic<bool> IsCheck = false;
	// 락 순서 그래프
	std::unordered_map<LockAddress, std::unordered_set<LockAddress>> _lockGraph;
	// 로그용
	std::unordered_map<LockAddress, const char*> _lockLog;
};
