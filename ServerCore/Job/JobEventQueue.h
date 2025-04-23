#pragma once
#include "Job.h"

/*-------------------------------------------------------
				JobEventQueue

- 우선순위별 처리
- 이벤트 방식의 Job 구현
--------------------------------------------------------*/

class JobEventQueue : public Singleton<JobEventQueue>
{
public:
	void Init() override;

	// 대기중인 작업 수
	uint16 GetPendingJobCount();

	// 즉시 처리용 작업 등록 (일반 콜백)
	void DoAsync(CallbackType&& callback, JobGroupType group = JobGroupType::System);

	// 시간 지연 처리용 작업 등록 (일반 콜백)
	void DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupType group = JobGroupType::System);

	// 멤버 함수를 이용한 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsync(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args... args);

	// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsyncAfter(uint64 delayMs, std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args... args);

	// 기본 타입-그룹 매핑 등록
	template<typename T>
	void RegisterTypeMapping(JobGroupType group);

	// 종료
	void Shutdown() override;

private:
	// 작업을 큐에 추가
	void Push(JobRef job);
	// 작업 큐에서 가져옴
	// LOCK은 호출하는 함수에서 건다.
	JobRef Pop(JobGroupType group);
	// 워커 스레드
	void WorkerThread(JobGroupType group);
	// 타 그룹에서 작업 훔쳐오기
	// - 그룹을 처리하는 스레드가 쉬고있을때 바쁜 타 스레드를 도와줌
	JobRef StealJob(JobGroupType group);
	// Job을 훔칠때 공정하게? 훔치기 위해 라운드 로빈 기반의 group 가져오기
	// JobPriority::Low는 제외한다.
	JobGroupType GetNextGroupIndex(JobGroupType myGroup);

private:
	// 우선순위 비교 연산자
	struct JobComparator
	{
		bool operator()(const JobRef& a, const JobRef& b)
		{
			// 1. 우선순위 비교 (높은 우선순위가 먼저)
			if (a->GetPriority() != b->GetPriority())
			{
				return static_cast<uint8>(a->GetPriority()) < static_cast<uint8>(b->GetPriority());
			}
			// 2. 실행 시간 비교 (빠른 시간이 먼저)
			if (a->GetExecuteTime() != b->GetExecuteTime())
			{
				return a->GetExecuteTime() > b->GetExecuteTime();
			}
			// 3. 생성 순서 비교 (먼저 생성된 것이 먼저)
			return a->GetCreationOrder() > b->GetCreationOrder();
		}
	};

private:
	USE_LOCK;
	// 우선순위 큐 타입
	using JobType = PriorityQueue<JobRef, Vector<JobRef>, JobComparator>;
	// 그룹별 작업 큐
	HashMap<JobGroupType, JobType> _groupJobs;
	// 조건변수(그룹 전체 관리)
	ConditionVariable _cv;
	// 스레드 실행중 플래그
	std::atomic<bool> _running = false;
};

// 멤버 함수를 이용한 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobEventQueue::DoAsync(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, 0, std::forward<Args>(args)...);
	Push(job);
}

// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobEventQueue::DoAsyncAfter(uint64 delayMs, std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, delayMs, std::forward<Args>(args)...);
	Push(job);
}

// 기본 타입-그룹 매핑 등록
template<typename T>
inline void JobEventQueue::RegisterTypeMapping(JobGroupType group)
{
	TypeToGroupMapper::RegisterType<T>(group);
}
