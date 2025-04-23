#pragma once
#include "Job/JobDefine.h"

/*-------------------------------------------------------
				ThreadManager

- 스레드의 생명 주기 관리
- JobGroupType별 스레드 관리
--------------------------------------------------------*/
class ThreadManager : public Singleton<ThreadManager>
{
public:
	// 초기화
	void Init() override;
	// 그룹별 스레드 생성 & 일감 투척
	void LaunchGroup(JobGroupType group, uint16 count, CallbackType jobCallback);
	// 그룹별 스레드 실행 완료 대기
	void JoinGroup(JobGroupType group);
	// TLS 데이터 제거
	void DeleteTLSData();
	// 전체 스레드 실행 완료 대기
	void JoinAll();
	// 활성 스레드 카운트 증가
	void PlusActiveThreadCount();
	// 활성 스레드 카운트 감소
	void MinusActiveThreadCount();
	// 종료
	void Shutdown() override;

private:
	// 스레드 이름 붙이기
	void SetThreadName(const std::string& name);

private:
	USE_LOCK;
	// 그룹별 스레드
	HashMap<JobGroupType, Vector<std::thread>> _groupThreads;
	// 활성화된 스레드 카운트
	std::atomic<uint16> _activeThreadCount;
};
