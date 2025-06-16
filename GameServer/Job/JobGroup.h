#pragma once

/*-------------------------------------------------------
					JobGroup

- 그룹명		: 사용할 JobGroupId 정의							/ JobGroup::Player
- 우선순위	: Job을 처리할 우선순위 지정(높을수록 우선처리)		/ Low, Normal, High
- 클래스명	: 실제 Job을 등록할 클래스명						/ Player, Dungeon, ...

JOB_GROUP에 등록시마다 각 그룹의 스레드가 생성된다.

Job 함수 호출시(DoAsync & DoAsyncAfter) 필요한 2가지 등록
1. Job 그룹 정의
   - 등록 형식: JOB_GROUP(그룹명, 우선순위)
   - ex) JOB_GROUP(JobGroup::Player, JobPriority::High);
2. .cpp 파일에 해당하는 클래스 include

ex)	JOB_GROUP(Player, High)
--------------------------------------------------------*/

class JobGroup
{
public:
	// JobGroup 등록
	void static Init();

public:
	// JobGroup 정적 멤버 정의
	static JobGroupId Character;
	static JobGroupId Enviroment;
};
