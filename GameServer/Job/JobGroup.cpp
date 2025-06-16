#include "pch.h"
#include "JobGroup.h"
#include "Character/CharacterObject.h"
#include "Map/BaseMap.h"

#include <typeindex>

#define JOB_GROUP(name, priority) \
    name = JobGroupMgr.RegisterContentGroup(#name, priority); \
	JobQ.MakeGroupThread(JobGroup::name);
	
/*----------------------------
		JobGroup
----------------------------*/

// 정적 멤버 정의
JobGroupId JobGroup::Character = JobGroups::Invalid;
JobGroupId JobGroup::Enviroment = JobGroups::Invalid;

// JobGroup 등록
void JobGroup::Init()
{
	// Character 그룹 추가
	// - Player 관련 모든 작업, 몬스터 전투 등
	JOB_GROUP(Character, JobPriority::High);

	// Enviroment 그룹 추가
	// - 채팅, 맵 업데이트 등
	JOB_GROUP(Enviroment, JobPriority::Normal);
}
