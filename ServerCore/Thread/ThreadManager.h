#pragma once

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
	// 스레드 생성 & 일감 투척
	void Launch(const std::string& threadName, uint16 count, CallbackType jobCallback);
	// 그룹별 스레드 생성 & 일감 투척
	void LaunchGroup(JobGroupId groupId, uint16 count, CallbackType jobCallback);
	// TLS 데이터 제거
	void DeleteTLSData();
	// 전체 스레드 실행 완료 대기
	void JoinAll();
	// 종료
	void Shutdown() override;

private:
	// 스레드 이름 붙이기
	void SetThreadName(const std::string& name);

private:
	USE_LOCK;
	// 전체 스레드
	Vector<std::thread> _threads;
};
