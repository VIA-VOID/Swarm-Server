#pragma once

/*-------------------------------------------------------
					JobGroup

- 그룹명	: 사용할 JobGroupId 정의

JOB_GROUP에 등록시마다 각 그룹의 스레드가 생성된다.
- 등록 형식: JOB_GROUP(그룹명)
- ex) JOB_GROUP(System);
--------------------------------------------------------*/

class JobGroup
{
public:
	// JobGroup 등록
	void static Init();

public:
	// JobGroup 정적 멤버 정의
	static JobGroupId Social;
	static JobGroupId Timer;
};
