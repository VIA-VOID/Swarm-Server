#pragma once
#include "NetworkDefine.h"
#include "Network/RecvBuffer.h"
#include "Network/SendBuffer.h"

class ServerCoreService;

/*-------------------------------------------------------
					Session

- 클라이언트 정보 관리
- Send, Recv
--------------------------------------------------------*/

class Session
{
public:
	Session();
	~Session();

	// 초기화
	bool Init(SOCKET socket, HANDLE iocpHandle);
	// 종료
	void Close();
	// Accept전 세션에 소켓, 서비스 설정
	void PreAccept(SOCKET socket, ServerCoreService* service);
	// WSASend 실행전 유효성, 버퍼 할당
	void Send(const BYTE* data, int32 len);
	// WSARecv 실행
	void ProcessRecv();
	// WSASend 실행
	void ProcessSend();
	// Recv 완료 & 패킷처리
	void OnRecvCompleted(int32 bytesTransferred);
	// Send 완료 & 남은 데이터 있을시 이어서 전송
	void OnSendCompleted(int32 bytesTransferred, OverlappedEx* overlappedEx);
	// 타임아웃 체크
	bool IsTimeout(std::chrono::seconds timeout = TIMEOUT_SECONDS);

	// getter -
	// 세션 ID 얻기
	SessionID GetSessionID();
	// 소켓 가져오기
	SOCKET GetSocket();

private:
	// 로그 찍기
	// ERROR, WARNING
	void LogError(const std::wstring& msg, const int32 errorCode, const LogType type = LogType::Error);

private:
	USE_LOCK;
	// 세션 정보
	SessionID _sessionID;
	SOCKET _socket;
	SOCKADDR_IN _clientAddress;
	TimePoint _lastRecvTime;
	TimePoint _connectedTime;
	std::atomic<bool> _connected;

	// I/O 작업 관련
	HANDLE _iocpHandle;
	OverlappedEx _recvOverlappedEx;
	OverlappedEx _sendOverlappedEx;

	// 버퍼 관련
	RecvBuffer _recvBuffer;
	Queue<SendBuffer*> _sendQueue;
	std::atomic<bool> _sending;

	// 서비스
	ServerCoreService* _service;
};
