#include "pch.h"
#include "JobEventQueue.h"
#include "Job.h"

/*----------------------------
		JobEventQueue
----------------------------*/

// 초기화
void JobEventQueue::Init()
{
	_running.store(true, std::memory_order::memory_order_relaxed);

	// 스레드 할당
	for (auto& groupPriority : GROUP_PRIORITY)
	{
		JobGroupType group = groupPriority.first;
		auto& pair = groupPriority.second;
		JobPriority priority = pair.first;
		uint16 threadCount = pair.second;

		ThreadMgr.LaunchGroup(group, threadCount, [this, group]()
			{
				WorkerThread(group);
			}
		);
	}

	LOG_SYSTEM(L"JobEventQueue instance initialized");
}

// 대기중인 작업 수
uint16 JobEventQueue::GetPendingJobCount()
{
	uint16 count = 0;
	for (const auto& pair : _groupJobs)
	{
		count += static_cast<uint16>(pair.second.size());
	}
	return count;
}

// 즉시 처리용 작업 등록 (일반 콜백)
void JobEventQueue::DoAsync(CallbackType&& callback, JobGroupType group /*= JobGroupType::System*/)
{
	Job* job = Job::Allocate(std::move(callback), group, 0);
	Push(job);
}

// 시간 지연 처리용 작업 등록 (일반 콜백)
void JobEventQueue::DoAsyncAfter(uint64 delayMs, CallbackType&& callback, JobGroupType group /*= JobGroupType::System*/)
{
	Job* job = Job::Allocate(std::move(callback), group, delayMs);
	Push(job);
}

// 종료
void JobEventQueue::Shutdown()
{
	// 모든 워커 스레드 종료 대기
	_running.store(false);
	// 모든 대기 스레드 깨우기
	_cv.notify_all();
}

// 작업을 큐에 추가
void JobEventQueue::Push(Job* job)
{
	JobGroupType group = job->GetGroup();
	{
		LOCK_GUARD;
		_groupJobs[group].push(job);
	}
	// 우선순위에 따라 스레드 깨우기
	if (job->GetPriority() == JobPriority::Low)
	{
		// 하나만
		_cv.notify_one();
	}
	else
	{
		// 전체
		_cv.notify_all();
	}
}

// 작업 큐에서 가져옴
// LOCK은 호출하는 함수에서 건다.
Job* JobEventQueue::Pop(JobGroupType group)
{
	// 우선순위 큐
	auto& groupQueue = _groupJobs[group];
	if (groupQueue.empty() == false)
	{
		Job* job = groupQueue.top();
		groupQueue.pop();
		return job;
	}

	return nullptr;
}

// 워커 스레드
void JobEventQueue::WorkerThread(JobGroupType group)
{
	while (_running.load())
	{
		Job* job = nullptr;
		// 일감이 있는지 확인하는 flag
		bool hasJob = false;
		{
			UNIQUE_LOCK_GUARD;
			// cv 대기
			_cv.wait(ulockGuard, [this, group]()
				{
					// 해당 그룹에 실행 가능한 작업이 있는지 확인
					bool hasExecuteWork = false;
					auto& groupQueue = _groupJobs[group];
					if (groupQueue.empty() == false && groupQueue.top()->IsExecute())
					{
						hasExecuteWork = true;
					}

					// 다른 그룹에 실행 가능한 작업이 있는지 확인
					bool hasOtherExecuteWork = false;
					for (const auto& pair : _groupJobs)
					{
						if (pair.first != group && pair.second.empty() == false)
						{
							auto& otherGroupQueue = pair.second;
							if (otherGroupQueue.empty() == false && otherGroupQueue.top()->IsExecute())
							{
								hasOtherExecuteWork = true;
								break;
							}
						}
					}
					// 종료 신호 or 실행할 작업이 있다면 깨어난다.
					return (_running.load() == false) || hasExecuteWork || hasOtherExecuteWork;
				}
			); // _cv.wait 끝

			// wait동안 바뀔수 있으니 확인
			if (_running.load() == false)
			{
				break;
			}

			// 자신의 그룹에서 작업 확인
			auto& groupQueue = _groupJobs[group];
			if (groupQueue.empty() == false && groupQueue.top()->IsExecute())
			{
				job = groupQueue.top();
				groupQueue.pop();
			}
			else
			{
				// 자신의 그룹에 실행 가능한 작업이 없다면 다른 그룹에서 훔쳐온다.
				job = StealJob(group);
			}

			// job이 있음
			if (job != nullptr)
			{
				hasJob = true;
			}
		} // UNIQUE_LOCK_GUARD 종료

		// job이 있음
		if (hasJob)
		{
			// 작업 실행
			job->Execute();
			// 완료후 해제
			Job::Release(job);
		}

	} // while 종료
}

// 타 그룹에서 작업 훔쳐오기
// - 그룹을 처리하는 스레드가 쉬고있을때 바쁜 타 스레드를 도와줌
Job* JobEventQueue::StealJob(JobGroupType group)
{
	// 훔칠 group 가져오기
	JobGroupType groupType = GetNextGroupIndex(group);
	auto& groupQueue = _groupJobs[group];
	// 작업이 남아있다면 확인
	if (groupQueue.empty() == false)
	{
		Job* job = groupQueue.top();
		// 실행가능한 작업만 가져온다
		if (job->IsExecute())
		{
			groupQueue.pop();
			return job;
		}
	}

	return nullptr;
}

// Job을 훔칠때 공정하게? 훔치기 위해 라운드 로빈 기반의 group 가져오기
// JobPriority::Low는 제외한다.
JobGroupType JobEventQueue::GetNextGroupIndex(JobGroupType myGroup)
{
	for (const auto& groupPriority : GROUP_PRIORITY)
	{
		JobGroupType group = groupPriority.first;
		auto& pair = groupPriority.second;
		// 내 그룹이 아닌것들만 가져온다.
		if (group == myGroup)
		{
			continue;
		}
		// 낮은 작업들은 굳이 훔치지 않음
		if (pair.first == JobPriority::Low)
		{
			continue;
		}
		// 이전에 처리한 그룹외의 것을 가져옴
		if (group == static_cast<JobGroupType>(LStealJobIndex))
		{
			continue;
		}
		LStealJobIndex = static_cast<uint16>(group);
		return group;
	}
	// 못훔쳤으면 자기그룹
	return myGroup;
}
