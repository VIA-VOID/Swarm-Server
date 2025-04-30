#pragma once
#include "NetworkDefine.h"

class ServerCoreService;

/*-------------------------------------------------------
				IocpServer

- WorkerThraed, AcceptThread 생성
- Iocp 네트워크 서비스
--------------------------------------------------------*/

class IocpServer
{
public:
	IocpServer();
	// 종료
	void Shutdown();
	// IocpServer 시작
	// Accept & Worker Thread 생성
	bool Start(uint16 port);
	// 서비스 연결
	void ConnectService(ServerCoreService* service);

private:
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	bool InitIocp();
	// acceptEx 함수 로딩
	bool WSAIoctlAcceptEx();
	// Accept 요청
	void RequestAccept();
	// AcceptEx 실행
	void ProcessAccept(Session* session);
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
	void LogError(const std::wstring& msg, const int32 errorCode);

private:
	LPFN_ACCEPTEX _acceptEx;
	std::atomic<bool> _running;
	SOCKET _listenSocket;
	HANDLE _iocpHandle;
	ServerCoreService* _service;
};
