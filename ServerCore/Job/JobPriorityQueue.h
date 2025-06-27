#pragma once
#include "Job.h"

/*-------------------------------------------------------
				JobPriorityQueue

- 우선순위별 처리
- 그룹 당 하나의 스레드, 지정한 프레임동안 Job 처리
--------------------------------------------------------*/

class JobPriorityQueue : public Singleton<JobPriorityQueue>
{
public:
	void Init() override;
	// 즉시 처리용 작업 등록 (일반 콜백)
	void DoAsync(CallbackType&& callback, JobGroupId groupId = JobGroups::System);
	// 시간 지연 처리용 작업 등록 (일반 콜백)
	void DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupId groupId = JobGroups::System);
	// 멤버 함수를 이용한 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsync(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args... args);
	// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
	template <typename T, typename Ret, typename... Args>
	void DoAsyncAfter(uint64 delayMs, std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args... args);
	// 종료
	void Shutdown() override;
	// 스레드 등록
	void MakeGroupThread(JobGroupId groupId);

private:
	// 작업을 큐에 추가
	void Push(const JobRef& job);
	// 작업 큐에서 가져옴
	// LOCK은 호출하는 함수에서 건다.
	JobRef Pop(JobGroupId groupId);
	// 워커 스레드
	void WorkerThread(JobGroupId group);

private:
	// 우선순위 비교 연산자
	struct JobComparator
	{
		bool operator()(const JobRef& a, const JobRef& b)
		{
			// 1. 실행 시간 비교 (빠른 시간이 먼저)
			if (a->GetExecuteTime() != b->GetExecuteTime())
			{
				return a->GetExecuteTime() > b->GetExecuteTime();
			}
			// 2. 생성 순서 비교 (먼저 생성된 것이 먼저)
			return a->GetCreationOrder() > b->GetCreationOrder();
		}
	};

private:
	// 우선순위 큐 타입
	using JobType = PriorityQueue<JobRef, Vector<JobRef>, JobComparator>;
	struct GroupJobQueue
	{
		JobType jobQueue;
		USE_LOCK;
	};
	// 그룹별 작업 큐
	HashMap<JobGroupId, GroupJobQueue> _groupJobs;
	// 스레드 실행 중 플래그
	HashMap<JobGroupId, std::atomic<bool>> _groupRunning;
};

// 멤버 함수를 이용한 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobPriorityQueue::DoAsync(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, 0, std::forward<Args>(args)...);
	Push(job);
}

// 멤버 함수를 이용한 시간 지연 처리용 작업 등록
template<typename T, typename Ret, typename ...Args>
inline void JobPriorityQueue::DoAsyncAfter(uint64 delayMs, std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args ...args)
{
	JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, delayMs, std::forward<Args>(args)...);
	Push(job);
}
