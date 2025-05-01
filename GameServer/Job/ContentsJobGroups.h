#pragma once
#include <typeindex>

// 컨텐츠 Job 그룹 ID
namespace JobGroups
{
	// 그룹 ID 선언
#define JOB_GROUP(name, count, priority, isInit) extern JobGroupId name;
#define CLASS_TO_JOB(className, groupName)

#include "ContentsJob.txt"

#undef CLASS_TO_JOB
#undef JOB_GROUP

	// 컨텐츠 그룹 초기화
	void Init();
	// Job 스레드 생성 요청
	void CreateThreadsForGroups();
}
