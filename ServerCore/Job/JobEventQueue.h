#pragma once
#include "Job.h"

/*-------------------------------------------------------
				JobEventQueue

- 우선순위별 처리
- 그룹 당 하나의 스레드, 이벤트 방식의 Job 처리
--------------------------------------------------------*/

class JobEventQueue : public Singleton<JobEventQueue>
{
public:
	void Init() override;
	// 즉시 처리용 작업 등록 (일반 콜백)
	void DoAsync(CallbackType&& callback, JobGroupId groupId = JobGroups::System);
	// 시간 지연 처리용 작업 등록 (일반 콜백)
	void DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupId groupId = JobGroups::System);
	// 멤버 함수를 이용한 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsync(T* owner, Ret(T::* memFunc)(Args...), Args... args);
	// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsyncAfter(uint64 delayMs, T* owner, Ret(T::* memFunc)(Args...), Args... args);
	// 종료
	void Shutdown() override;
	// 그룹별 스레드 생성
	void RegisterThreadsForGroup(JobGroupId groupId);

private:
	// 작업을 큐에 추가
	void Push(Job* job);
	// 작업 큐에서 가져옴
	// LOCK은 호출하는 함수에서 건다.
	Job* Pop(JobGroupId groupId);
	// 스레드 추가
	void AddThread(JobGroupId groupId);
	// 워커 스레드
	void WorkerThread(JobGroupId group);

private:
	// 우선순위 비교 연산자
	struct JobComparator
	{
		bool operator()(const Job* a, const Job* b)
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
	// 우선순위 큐 타입
	using JobType = PriorityQueue<Job*, Vector<Job*>, JobComparator>;
	struct GroupJobQueue
	{
		JobType jobQueue;
		USE_LOCK;
	};
	// 그룹별 작업 큐
	HashMap<JobGroupId, GroupJobQueue> _groupJobs;
	// 조건변수
	HashMap<JobGroupId, ConditionVariable> _groupCVs;
	// 스레드 실행 중 플래그
	HashMap<JobGroupId, std::atomic<bool>> _groupRunning;
};

// 멤버 함수를 이용한 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobEventQueue::DoAsync(T* owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	Job* job = ObjectPool<Job>::Allocate(owner, memFunc, 0, std::forward<Args>(args)...);
	Push(job);
}

// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobEventQueue::DoAsyncAfter(uint64 delayMs, T* owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	Job* job = ObjectPool<Job>::Allocate(owner, memFunc, delayMs, std::forward<Args>(args)...);
	Push(job);
}
