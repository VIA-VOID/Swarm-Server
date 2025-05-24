#pragma once
#include "Network/RecvBuffer.h"
#include "Network/SendBuffer.h"

class CoreService;

// 세션상태
enum class SessionState
{
	Active,			// 활성
	CloseRequest,	// 종료 요청
	Closed			// 완전 종료 - 메모리 해제
};

/*
	패킷구조
	[id][size][protobuf data]
	- id: 프로토콜 ID
	- size: 패킷 전체 크기(헤더 포함)
	- data: 직렬화된 protobuf 데이터
*/
struct PacketHeader
{
	uint16 id;
	uint16 size;
};

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
	// 세션에 소켓, 서비스 설정
	void PreInit(SOCKET socket, CoreService* service);
	// WSASend 실행전 유효성, 버퍼 할당
	void Send(const BYTE* data, int32 len);
	// Recv 완료 & 패킷처리
	void OnRecvCompleted(int32 bytesTransferred);
	// Send 완료 & 남은 데이터 있을시 이어서 전송
	void OnSendCompleted(int32 bytesTransferred);
	// 타임아웃 체크
	bool IsTimeout(std::chrono::seconds timeout = TIMEOUT_SECONDS);

	// getter -
	// 세션 ID 얻기
	SessionID GetSessionID();
	// 소켓 가져오기
	SOCKET GetSocket();
	// 세션 상태
	SessionState GetState();
	// ConnectContext 가져오기
	ConnectContext* GetConnectContext();
	// AcceptContext 가져오기
	AcceptContext* GetAcceptContext();
	// 세션 상태 변경
	void SetState(SessionState state);
	// 활성상태 여부
	bool IsActive();


private:
	// WSARecv 실행
	void ProcessRecv();
	// WSASend 실행
	void ProcessSend();
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

	// I/O 작업 관련
	HANDLE _iocpHandle;
	ConnectContext _connectContext;
	AcceptContext _acceptContext;
	RecvContext _recvContext;
	SendContext _sendContext;

	// 버퍼 관련
	Queue<SendBuffer*> _sendQueue;
	std::atomic<bool> _sending;

	// 세션 상태 관리
	std::atomic<SessionState> _state;
	TimePoint _closeRequestTime;

	// 서비스
	CoreService* _service;
};
