#include "pch.h"
#include "ContentsJobGroups.h"
#include "../Player.h"

namespace JobGroups
{
	// ID 변수 정의
#define JOB_GROUP(name, count, priority, isInit) JobGroupId name = JobGroups::Invalid;
#define CLASS_TO_JOB(className, groupName)

#include "ContentsJob.txt"

#undef CLASS_TO_JOB
#undef JOB_GROUP

	// 클래스-그룹 매핑 초기화
	void InitClassMappings()
	{
		// CLASS_TO_JOB 매크로에서 타입 매핑을 자동으로 등록
#define JOB_GROUP(name, count, priority, isInit)
#define CLASS_TO_JOB(className, groupName) \
	JobGroupMgr.SetTypeToGroup(std::type_index(typeid(className)), JobGroups::groupName);

#include "ContentsJob.txt"

#undef CLASS_TO_JOB
#undef JOB_GROUP
	}

	// 컨텐츠 그룹 초기화
	void Init()
	{
		// 모든 컨텐츠 그룹 등록
#define JOB_GROUP(name, count, priority, isInit) \
	name = JobGroupMgr.RegisterContentGroup(#name, count, JobPriority::priority, isInit);
#define CLASS_TO_JOB(className, groupName)

#include "ContentsJob.txt"

#undef CLASS_TO_JOB
#undef JOB_GROUP

		// 클래스-그룹 매핑 초기화
		InitClassMappings();
	}

	// Job 스레드 생성 요청
	void CreateThreadsForGroups()
	{
#define JOB_GROUP(name, count, priority, isInit) \
	JobQ.RegisterThreadsForGroup(name);
#define CLASS_TO_JOB(className, groupName)

#include "ContentsJob.txt"

#undef CLASS_TO_JOB
#undef JOB_GROUP
	}
}
