#pragma once
#include "Network/RecvBuffer.h"
#include "Network/SendBuffer.h"

class CoreService;

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

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session();
	virtual ~Session();

	// 초기화
	bool Init(SOCKET socket, HANDLE iocpHandle, bool createIOCP = true);
	// 종료
	void Close();
	// 세션에 소켓, 서비스 설정
	void PreInit(SOCKET socket, CoreService* service);
	// 비동기 송신 시작
	void Send(const SendBufferRef& sendBuffer);
	// Recv 완료 & 패킷처리
	void OnRecvCompleted(int32 bytesTransferred);
	// Send 완료 & 남은 데이터 있을시 이어서 전송
	void OnSendCompleted(int32 bytesTransferred);
	// 타임아웃 체크
	bool IsTimeout(std::chrono::seconds timeout = TIMEOUT_SECONDS) const;
	// RTT 업데이트
	void updateRoundTripTime(int32 roundTripTime);
	// PingCount 증가
	void IncreasePingCount();
	// PongCount 증가
	void IncreasePongCount();

	// getter -
	// 세션 ID 얻기
	SessionID GetSessionID() const;
	// 소켓 가져오기
	SOCKET GetSocket() const;
	// 유저 클래스 설정
	template <typename T>
	void SetPlayer(T* player);
	// 유저 클래스 얻기
	template <typename T>
	T* GetPlayer();
	// 유저 클래스 포인터 참조 정리
	void DetachPlayer();
	// 세션 close 여부
	bool IsClosed();
	// RTT 평균값 가져오기
	int32 GetRttAvg() const;
	// PingCount 가져오기
	uint64 GetPingCount() const;
	// PongCount 가져오기
	uint64 GetPongCount() const;

private:
	// 자원정리
	void CloseResource();
	// WSARecv 실행
	void ProcessRecv();
	// WSASend 실행
	void ProcessSend();
	// 로그 찍기
	// ERROR, WARNING
	void LogError(const std::string& msg, const int32 errorCode, const LogType type = LogType::Error);

private:
	USE_LOCK;
	
	// 세션 정보
	SessionID _sessionID;
	SOCKET _socket;
	SOCKADDR_IN _clientAddress;
	TimePoint _lastRecvTime;
	TimePoint _lastSendTime;
	TimePoint _connectedTime;
	std::atomic<bool> _isClosed;

	// I/O 작업 관련
	HANDLE _iocpHandle;
	RecvContext _recvContext;
	SendContext _sendContext;

	// 버퍼 관련
	Deque<SendBufferRef> _sendDeque;
	std::atomic<bool> _sending;

	// RTT
	int32 _avgRoundTripTime;
	Deque<int32> _rttDeque;

	// PING & PONG (HeartBeat)
	uint64 _pingCount;
	uint64 _pongCount;

	// 서비스
	CoreService* _service;

	// 유저클래스(플레이어)
	void* _playerClass;
};

// 유저 클래스 설정
template<typename T>
inline void Session::SetPlayer(T* player)
{
	_playerClass = static_cast<void*>(player);
}

// 유저 클래스 얻기
template<typename T>
inline T* Session::GetPlayer()
{
	if (_playerClass == nullptr)
	{
		return nullptr;
	}
	return static_cast<T*>(_playerClass);
}
