#include "pch.h"
#include "Job.h"

/*----------------------------
			Job
----------------------------*/

// 우선순위에 따라 즉시 처리용 생성자
Job::Job(CallbackType&& callback, JobGroupId groupId /*= JobGroups::System*/, uint64 delayMs /*= 0*/)
	: _callback(std::move(callback)),
	_groupId(groupId),
	_executeTime(delayMs > 0 ? NOW + std::chrono::milliseconds(delayMs) : NOW),
	_orderNum(GetNextOrderNum())
{
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

TimePoint Job::GetExecuteTime() const
{
	return _executeTime;
}

uint64 Job::GetCreationOrder() const
{
	return _orderNum;
}

JobGroupId Job::GetGroupId() const
{
	return _groupId;
}

// 작업 생성 순서를 위한 카운터
uint64 Job::GetNextOrderNum()
{
	static std::atomic<uint64> counter = 0;
	return counter.fetch_add(1, std::memory_order_relaxed);
}
