#include "pch.h"
#include "JobPriorityQueue.h"
#include "Job.h"

/*----------------------------
		JobPriorityQueue
----------------------------*/

// 초기화
void JobPriorityQueue::Init()
{
	// 스레드 할당
	const auto& allGroups = JobGroupMgr.GetAllGroups();
	for (const auto& pair : allGroups)
	{
		JobGroupId groupId = pair.first;
		MakeGroupThread(groupId);
	}
}

// 즉시 처리용 작업 등록 (일반 콜백)
void JobPriorityQueue::DoAsync(CallbackType&& callback, JobGroupId groupId /*= JobGroups::System*/)
{
	JobRef job = ObjectPool<Job>::MakeShared(std::move(callback), groupId, 0);
	Push(job);
}

// 시간 지연 처리용 작업 등록 (일반 콜백)
void JobPriorityQueue::DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupId groupId /*= JobGroups::System*/)
{
	JobRef job = ObjectPool<Job>::MakeShared(std::move(callback), groupId, delayMs);
	Push(job);
}

// 종료
void JobPriorityQueue::Shutdown()
{
	// 모든 그룹의 워커 스레드 종료
	for (auto& pair : _groupRunning)
	{
		JobGroupId groupId = pair.first;
		pair.second.store(false);
	}
}

// 스레드 등록
void JobPriorityQueue::MakeGroupThread(JobGroupId groupId)
{
	ASSERT_CRASH(groupId != JobGroups::Invalid);

	if (_groupRunning.find(groupId) != _groupRunning.end())
	{
		CRASH("already running thread");
	}
	// 그룹 상태 활성화
	_groupRunning[groupId].store(true, std::memory_order::memory_order_relaxed);
	// 그룹별 스레드 생성
	ThreadMgr.LaunchGroup(groupId, 1, [this, groupId]()
		{
			WorkerThread(groupId);
		}
	);
}

// 작업을 큐에 추가
void JobPriorityQueue::Push(const JobRef& job)
{
	JobGroupId groupId = job->GetGroupId();
	{
		GROUP_LOCK_GUARD(_groupJobs[groupId]);
		_groupJobs[groupId].jobQueue.push(job);
	}
}

// 작업 큐에서 가져옴
// LOCK은 호출하는 함수에서 건다.
JobRef JobPriorityQueue::Pop(JobGroupId groupId)
{
	// 우선순위 큐
	auto& groupQueue = _groupJobs[groupId].jobQueue;
	if (groupQueue.empty())
	{
		return nullptr;
	}
	JobRef job = groupQueue.top();
	if (job->IsExecute() == false)
	{
		return nullptr;
	}
	groupQueue.pop();
	return job;
}

// 워커 스레드
void JobPriorityQueue::WorkerThread(JobGroupId groupId)
{
	while (_groupRunning[groupId].load())
	{
		JobRef job = nullptr;
		bool hasJob = false;
		{
			GROUP_LOCK_GUARD(_groupJobs[groupId]);
			auto& jobQueue = _groupJobs[groupId].jobQueue;

			// 종료 확인
			if (_groupRunning[groupId].load() == false)
			{
				break;
			}
			// 작업 가져오기
			job = Pop(groupId);
			if (job != nullptr)
			{
				hasJob = true;
			}
		}
		
		// 작업 처리
		if (hasJob)
		{
			// 작업 실행
			job->Execute();
		}
		else
		{
			// 없으면 잠시 대기
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

	}
}
