#include "pch.h"
#include "JobGroup.h"
#include "Character/CharacterObject.h"

#include <typeindex>

#define JOB_GROUP(name, priority) \
    name = JobGroupMgr.RegisterContentGroup(#name, priority);

#define CLASS_TO_JOB(className, groupName) \
    JobGroupMgr.SetTypeToGroup(std::type_index(typeid(className)), groupName); \
    JobQ.RegisterThreadsForGroup(groupName);

/*----------------------------
		JobGroup
----------------------------*/

// 정적 멤버 정의
JobGroupId JobGroup::Player = UINT16_MAX;

// JobGroup 등록
void JobGroup::Init()
{
	JOB_GROUP(JobGroup::Player, JobPriority::High);
	CLASS_TO_JOB(CharacterObject, JobGroup::Player);
}
