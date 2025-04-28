#pragma once

/*-------------------------------------------------------
				IocpCore
--------------------------------------------------------*/

class IocpServer : public Singleton<IocpServer>
{
public:
	// 초기화
	void Init() override;
	// 종료
	void Shutdown() override;
	// IocpServer 시작
	// Accept & Worker Thread 생성
	bool Start(uint16 port);

private:
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	bool InitIocp();
	// acceptEx 함수 로딩
	bool WSAIoctlAcceptEx();
	// AcceptEx 요청
	void ProcessAccept(SessionRef session);
	// Accept 스레드
	void AcceptThread();
	// Accept 완료
	bool OnAcceptCompleted(OverlappedEx* overlappedEx);
	// GQCS 워커 스레드
	void IOWorkerThread();
	// 워커 스레드 깨우기
	void WakeUpIOWorkerThreads();
	// 세션 타임아웃 체크 & Job 재등록
	void CheckTickTimeout();
	// HeartBeat 작업 등록
	void StartHeartbeatTask();
	// 로그 찍기
	void LogError(const std::wstring& msg);

private:
	LPFN_ACCEPTEX _acceptEx;
	std::atomic<bool> _running;
	SOCKET _listenSocket;
	HANDLE _iocpHandle;
};
