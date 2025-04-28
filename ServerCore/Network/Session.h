#pragma once
#include "NetworkDefine.h"
#include "Network/RecvBuffer.h"
#include "Network/SendBuffer.h"

/*-------------------------------------------------------
					Session
--------------------------------------------------------*/

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session();
	virtual ~Session();

	// 초기화
	bool Init(SOCKET socket, HANDLE iocpHandle);
	// 종료
	void Close();
	// 소켓만 먼저 설정
	void PreAccept(SOCKET socket);
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

	// 컨텐츠 로직에서 구현하여 사용
	virtual void OnConnected() = 0;
	virtual void OnDisconnected() = 0;
	virtual void OnRecv(const BYTE* buffer, int32 len) = 0;
	virtual void OnSend(int32 len) = 0;

private:
	// 로그 찍기
	// ERROR, WARNING
	void LogError(const std::wstring& msg, const LogType type = LogType::Error);

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
	Queue<SendBufferRef> _sendQueue;
	std::atomic<bool> _sending;
};
