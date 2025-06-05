#pragma once

/*-------------------------------------------------------
					JobGroup

- 그룹명		: 사용할 JobGroupId 정의							/ JobGroup::Player
- 우선순위	: Job을 처리할 우선순위 지정(높을수록 우선처리)		/ Low, Normal, High
- 클래스명	: 실제 Job을 등록할 클래스명						/ Player, Dungeon, ...

Job 함수 호출시(DoAsync & DoAsyncAfter) 필요한 3가지 등록
1. Job 그룹 정의
   - 등록 형식: JOB_GROUP(그룹명, 우선순위)
   - ex) JOB_GROUP(JobGroup::Player, JobPriority::High);

2. 등록한 JobGroup(JobGroupId)와 클래스 매핑
   - 등록 형식: CLASS_TO_JOB(클래스명, 그룹명)
   - ex) CLASS_TO_JOB(Player, Player)

3. .cpp 파일에 해당하는 클래스 include

그룹명 1개당 여러 클래스 등록가능
ex)	JOB_GROUP(Player, High)
	CLASS_TO_JOB(PlayerManager, Player)
	CLASS_TO_JOB(InventorySystem, Player)
	CLASS_TO_JOB(PlayerSkill, Player)
--------------------------------------------------------*/

class JobGroup
{
public:
	// JobGroup 등록
	void static Init();

public:
	// JobGroup 정적 멤버 정의
};
