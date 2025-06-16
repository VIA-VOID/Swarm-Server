#pragma once
#include "JobGroupType.h"
#include "Job/JobGroupManager.h"

/*-------------------------------------------------------
					Job

- JobGroupType별 Job 분리
- JobPriority 단위별 우선처리
	- 일감에 우선순위를 두고, 우선처리
--------------------------------------------------------*/
class Job
{
public:
	// 멤버 함수 포인터를 이용한 생성자
	template <typename T, typename Ret, typename... Args>
	Job(T* owner, Ret(T::* memFunc)(Args...), JobGroupId groupId = JobGroups::System, uint64 delayMs = 0, Args... args);
	// 우선순위에 따라 즉시 처리용 생성자
	Job(CallbackType&& callback, JobGroupId groupId = JobGroups::System, uint64 delayMs = 0);
	// 작업 실행
	void Execute();
	// 실행가능여부
	bool IsExecute();
	// getter
	JobPriority GetPriority() const;
	JobGroupId GetGroupId() const;
	TimePoint GetExecuteTime() const;
	uint64 GetCreationOrder() const;

private:
	// 작업 생성 순서를 위한 카운터
	static uint64 GetNextOrderNum();

private:
	// 콜백 functional 함수
	CallbackType _callback;
	// 작업 우선순위
	JobPriority _priority;
	// 작업 그룹
	JobGroupId _groupId;
	// 실행 시간
	TimePoint _executeTime;
	// 작업 생성 순서
	uint64 _orderNum;
};

// 멤버 함수 포인터를 이용한 생성자
template<typename T, typename Ret, typename ...Args>
inline Job::Job(T* owner, Ret(T::* memFunc)(Args...), JobGroupId groupId, uint64 delayMs, Args ...args)
	: _priority(JobPriority::Normal),
	_groupId(groupId),
	_executeTime(delayMs > 0 ? NOW + std::chrono::milliseconds(delayMs) : NOW),
	_orderNum(GetNextOrderNum())
{
	_callback = [owner, memFunc, args...]()
		{
			((*owner).*memFunc)(args...);
		};

	// 우선순위 지정
	const JobGroupType* groupInfo = JobGroupMgr.GetGroupInfo(_groupId);
	if (groupInfo)
	{
		_priority = groupInfo->GetGroupPriority();
	}
}
