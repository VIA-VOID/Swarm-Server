#pragma once

class CoreService;

/*-------------------------------------------------------
				IocpCore

- WorkerThraed, AcceptThread 생성
- Iocp 네트워크 서비스
--------------------------------------------------------*/

class IocpCore
{
public:
	IocpCore();
	// 종료
	void Shutdown();
	// IocpCore 시작
	// Accept & Worker Thread 생성
	bool Start(uint16 port);
	// 서버에 연결
	bool Connect(const std::string& address, uint16 port, uint16 maxWorkerThreadNum);
	// 서비스 연결
	void ConnectService(CoreService* service);
	
private:
	// 소켓 생성 및 Bind
	bool CreateSocketAndBind(SOCKET& socket, uint16 port);
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	bool InitIocp(SOCKET socket);
	// connectEx 함수 로딩
	bool WSAIoctlConnectEx(SOCKET socket);
	// 워커 스레드 시작
	void StartWorkerThreads(uint16 maxWorkerThreadNum);
	// connectEx 실행
	// 서버 연결 시도
	void ProcessConnect(SessionRef session, const std::string& address, uint16 port);
	// connect 완료
	bool OnConnectCompleted(OverlappedEx* overlappedEx);
	// acceptEx 함수 로딩
	bool WSAIoctlAcceptEx();
	// Accept 요청
	void RequestAccept();
	// Accept 재요청
	void ReRequestAccept();
	// AcceptEx 실행
	void ProcessAccept(SessionRef session);
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
	void LogError(const std::string& msg, const int32 errorCode);

private:
	LPFN_ACCEPTEX _acceptEx;
	LPFN_CONNECTEX _connectEx;
	std::atomic<bool> _running;
	SOCKET _listenSocket;
	HANDLE _iocpHandle;
	CoreService* _service;
};
