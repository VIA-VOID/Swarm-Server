#include "pch.h"
#include "JobEventQueue.h"
#include "Job.h"

/*----------------------------
		JobEventQueue
----------------------------*/

// 초기화
void JobEventQueue::Init()
{
	// 스레드 할당
	const auto& allGroups = JobGroupMgr.GetAllGroups();
	for (const auto& pair : allGroups)
	{
		JobGroupId groupId = pair.first;
		_groupRunning[groupId].store(true, std::memory_order::memory_order_relaxed);
		AddThread(groupId);
	}
}

// 즉시 처리용 작업 등록 (일반 콜백)
void JobEventQueue::DoAsync(CallbackType&& callback, JobGroupId groupId /*= JobGroups::System*/)
{
	Job* job = Job::Allocate(std::move(callback), groupId, 0);
	Push(job);
}

// 시간 지연 처리용 작업 등록 (일반 콜백)
void JobEventQueue::DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupId groupId /*= JobGroups::System*/)
{
	Job* job = Job::Allocate(std::move(callback), groupId, delayMs);
	Push(job);
}

// 종료
void JobEventQueue::Shutdown()
{
	// 모든 그룹의 워커 스레드 종료
	for (auto& pair : _groupRunning)
	{
		JobGroupId groupId = pair.first;
		pair.second.store(false);

		// 그룹의 스레드 깨우기
		if (_groupCVs.find(groupId) != _groupCVs.end())
		{
			_groupCVs[groupId].notify_all();
		}
	}
}

// 그룹별 스레드 생성
void JobEventQueue::RegisterThreadsForGroup(JobGroupId groupId)
{
	const JobGroupType* groupType = JobGroupMgr.GetGroupInfo(groupId);
	if (groupType == nullptr)
	{
		return;
	}
	// 스레드 생성
	AddThread(groupId);
}

// 작업을 큐에 추가
void JobEventQueue::Push(Job* job)
{
	JobGroupId groupId = job->GetGroupId();
	{
		GROUP_LOCK_GUARD(_groupJobs[groupId]);
		_groupJobs[groupId].jobQueue.push(job);
	}
	// 그룹의 스레드 깨우기
	_groupCVs[groupId].notify_one();
}

// 작업 큐에서 가져옴
// LOCK은 호출하는 함수에서 건다.
Job* JobEventQueue::Pop(JobGroupId groupId)
{
	// 우선순위 큐
	auto& groupQueue = _groupJobs[groupId].jobQueue;
	if (groupQueue.empty())
	{
		return nullptr;
	}
	Job* job = groupQueue.top();
	if (job->IsExecute() == false)
	{
		return nullptr;
	}
	groupQueue.pop();
	return job;
}

// 스레드 추가
void JobEventQueue::AddThread(JobGroupId groupId)
{
	// 그룹 상태 활성화
	_groupRunning[groupId].store(true, std::memory_order::memory_order_relaxed);
	// 그룹별 스레드 생성
	std::string groupName = JobGroupMgr.GetGroupName(groupId);
	ThreadMgr.LaunchGroup(groupId, 1, [this, groupId]()
		{
			WorkerThread(groupId);
		}
	);
}

// 워커 스레드
void JobEventQueue::WorkerThread(JobGroupId groupId)
{
	while (_groupRunning[groupId].load())
	{
		Job* job = nullptr;
		// 일감이 있는지 확인하는 flag
		bool hasJob = false;
		{
			GROUP_UNIQUE_LOCK(_groupJobs[groupId]);
			// cv 대기
			_groupCVs[groupId].wait(ulockGuard, [this, groupId]()
				{
					auto& jobQueue = _groupJobs[groupId].jobQueue;
					// 종료 신호 or 실행할 작업이 있다면 깨어난다.
					return (_groupRunning[groupId].load() == false) ||
						(jobQueue.empty() == false && jobQueue.top()->IsExecute());
				}
			);
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
			// 완료후 해제
			Job::Release(job);
		}

	} // while 종료
}
