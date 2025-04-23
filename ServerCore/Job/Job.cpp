#include "pch.h"
#include "Job.h"

/*----------------------------
			Job
----------------------------*/

// 우선순위에 따라 즉시 처리용 생성자
Job::Job(CallbackType&& callback, JobGroupType group /*= JobGroupType::System*/, uint64 delayMs)
	: _callback(std::move(callback)),
	_group(group),
	_executeTime(delayMs > 0 ? NOW + std::chrono::milliseconds(delayMs) : NOW),
	_orderNum(GetNextOrderNum())
{
	// 우선순위 지정
	auto it = GROUP_PRIORITY.find(_group);
	if (it != GROUP_PRIORITY.end())
	{
		_priority = it->second.first;
	}
}

// 작업 실행
void Job::Execute()
{
	if (_callback)
	{
		_callback();
	}
}

// 실행가능여부
bool Job::IsExecute()
{
	return NOW >= _executeTime;
}

JobPriority Job::GetPriority()
{
	return _priority;
}

TimePoint Job::GetExecuteTime()
{
	return _executeTime;
}

uint64 Job::GetCreationOrder()
{
	return _orderNum;
}

JobGroupType Job::GetGroup()
{
	return _group;
}

// 작업 생성 순서를 위한 카운터
uint64 Job::GetNextOrderNum()
{
	static std::atomic<uint64> counter = 0;
	return counter.fetch_add(1, std::memory_order_relaxed);
}
