#pragma once

/*-------------------------------------------------------
					Job

- JobGroupType별 Job 분리
	- 게임이 크지 않아서 객체 단위로 grouping
- JobPriority 단위별 우선처리
	- 일감에 우선순위를 두고, 우선처리
--------------------------------------------------------*/
class Job
{
public:
	// 멤버 함수 포인터를 이용한 생성자
	template <typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), uint64 delayMs = 0, Args... args);
	// 우선순위에 따라 즉시 처리용 생성자
	Job(CallbackType&& callback, JobGroupType group = JobGroupType::System, uint64 delayMs = 0);
	// 작업 실행
	void Execute();
	// 실행가능여부
	bool IsExecute();
	// getter
	JobPriority GetPriority();
	TimePoint GetExecuteTime();
	uint64 GetCreationOrder();
	JobGroupType GetGroup();

private:
	// 작업 생성 순서를 위한 카운터
	static uint64 GetNextOrderNum();

private:
	// 콜백 functional 함수
	CallbackType _callback;
	// 작업 우선순위
	JobPriority _priority;
	// 작업 그룹
	JobGroupType _group;
	// 실행 시간
	TimePoint _executeTime;
	// 작업 생성 순서
	uint64 _orderNum;
};

// 멤버 함수 포인터를 이용한 생성자
template<typename T, typename Ret, typename ...Args>
inline Job::Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), uint64 delayMs, Args ...args)
	: _priority(JobPriority::Normal),
	_group(TypeToGroupMapper::GetGroupType<T>()),
	_executeTime(delayMs > 0 ? NOW + std::chrono::milliseconds(delayMs) : NOW),
	_orderNum(GetNextOrderNum())
{
	_callback = [owner, memFunc, args...]()
		{
			((*owner).*memFunc)(args...);
		};

	// 우선순위 지정
	auto it = GROUP_PRIORITY.find(_group);
	if (it != GROUP_PRIORITY.end())
	{
		_priority = it->second.first;
	}
}
