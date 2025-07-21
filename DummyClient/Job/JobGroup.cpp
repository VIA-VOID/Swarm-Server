#include "pch.h"
#include "JobGroup.h"

#define JOB_GROUP(name) \
    name = JobGroupMgr.RegisterContentGroup(#name); \
	JobQ.MakeGroupThread(JobGroup::name);

/*----------------------------
		JobGroup
----------------------------*/

// 정적 멤버 정의
JobGroupId JobGroup::Social = JobGroups::Invalid;
JobGroupId JobGroup::Timer = JobGroups::Invalid;

// JobGroup 등록
void JobGroup::Init()
{
	JOB_GROUP(Social);
	JOB_GROUP(Timer);
}
