#include "pch.h"
#include "Session.h"
#include "SessionManager.h"

/*----------------------------
		Session
----------------------------*/

Session::Session()
	: _socket(INVALID_SOCKET), _iocpHandle(INVALID_HANDLE_VALUE), _sessionID(SessionID::Generate()),
	_clientAddress({}), _lastRecvTime(NOW), _lastSendTime(NOW), _connectedTime(NOW), _service(nullptr), _playerClass(nullptr)
{
	_recvContext.type = NetworkIOType::Recv;
	_sendContext.type = NetworkIOType::Send;

	_isClosed.store(false, std::memory_order_relaxed);
	_sending.store(false, std::memory_order_relaxed);
}

Session::~Session()
{
	if (_isClosed.load() == false)
	{
		// 자원 해제
		CloseResource();
	}
}

// 초기화
bool Session::Init(SOCKET socket, HANDLE iocpHandle)
{
	_socket = socket;
	_lastRecvTime = NOW;
	_lastSendTime = NOW;
	_connectedTime = NOW;
	_iocpHandle = iocpHandle;

	// IOCP에 등록
	::CreateIoCompletionPort(reinterpret_cast<HANDLE>(_socket), _iocpHandle, reinterpret_cast<ULONG_PTR>(this), 0);

	// 클라이언트 주소 정보 저장
	socklen_t addrLen = sizeof(_clientAddress);
	if (::getpeername(_socket, reinterpret_cast<SOCKADDR*>(&_clientAddress), &addrLen) == SOCKET_ERROR)
	{
		// 주소 저장 실패시 세션 종료
		int32 errorCode = ::WSAGetLastError();
		LogError("getpeername 실패", errorCode, LogType::Warning);
		return false;
	}
	// 비동기 수신 시작
	ProcessRecv();
	// 접속 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnConnected(shared_from_this());

	return true;
}

// 세션 종료
void Session::Close()
{
	if (_isClosed.exchange(true))
	{
		return;
	}
	_sending.store(false);
	
	std::weak_ptr<Session> weakSelf = weak_from_this();
	SessionRef self = weakSelf.lock();
	
	// 연결 종료 이벤트 호출
	if (_service != nullptr && self != nullptr)
	{
		_service->OnDisconnected(self);
	}

	CloseResource();
}

// 세션에 소켓, 서비스 설정
void Session::PreInit(SOCKET socket, CoreService* service)
{
	_socket = socket;
	_service = service;
	_lastRecvTime = NOW;
	_lastSendTime = NOW;
	_connectedTime = NOW;
}

// 비동기 송신 시작
void Session::Send(const SendBufferRef& sendBuffer)
{
	if (_isClosed.load())
	{
		return;
	}

	// 송신 데이터 삽입
	bool processSend = false;
	{
		LOCK_GUARD;
		_sendDequeue.push_back(sendBuffer);

		if (_sending.exchange(true) == false)
		{
			processSend = true;
		}
	}

	// 비동기 송신 시작
	if (processSend)
	{
		ProcessSend();
	}
}

// 유저 클래스 포인터 참조 정리
void Session::DetachPlayer()
{
	_playerClass = nullptr;
}

// 세션 close 여부
bool Session::IsClosed()
{
	return _isClosed.load();
}

// 자원정리
void Session::CloseResource()
{
	// 소켓 닫기
	if (_socket != INVALID_SOCKET)
	{
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
	// 송신 큐 비우기
	{
		LOCK_GUARD;
		_sendContext.buffers.clear();
		_sendDequeue.clear();
	}
}

// WSARecv 실행
void Session::ProcessRecv()
{
	// 초기화
	ZeroMemory(&_recvContext.overlapped, sizeof(_recvContext.overlapped));

	// wsabuf 세팅
	WSABUF wsabuf;
	_recvContext.recvBuffer.GetWSABUF(wsabuf);

	// 수신 가능한 공간이 없으면 버퍼 정리
	if (wsabuf.len == 0)
	{
		// 정리 시도 후 다시 wsabuf 세팅
		_recvContext.recvBuffer.Compact();
		_recvContext.recvBuffer.GetWSABUF(wsabuf);
		
		// 그래도 없으면 세션종료
		if (wsabuf.len == 0)
		{
			LOG_ERROR("RecvBuffer 공간 부족");
			Close();
			return;
		}
	}

	DWORD recvBytes = 0;
	DWORD flags = 0;
	if (::WSARecv(_socket, &wsabuf, 1, &recvBytes, &flags, reinterpret_cast<OVERLAPPED*>(&_recvContext), NULL) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 세션종료
			int32 errorCode = ::WSAGetLastError();
			LogError("WSARecv 실패, 해당 세션 종료", errorCode);
			Close();
		}
	}
}

// WSASend 실행
void Session::ProcessSend()
{
	// 한 번에 최대 MAX_SEND_BUFFER_COUNT개의 버퍼를 보냄
	Vector<SendBufferRef> sendBuffers;
	sendBuffers.reserve(MAX_SEND_BUFFER_COUNT);
	
	WSABUF wsabufs[MAX_SEND_BUFFER_COUNT];
	uint32 sendCount = 0;
	
	// 보내야할 패킷 꺼내기
	{
		LOCK_GUARD;

		ZeroMemory(&_sendContext.overlapped, sizeof(_sendContext.overlapped));
		
		while (_sendDequeue.empty() == false && MAX_SEND_BUFFER_COUNT > sendCount)
		{
			SendBufferRef buffer = _sendDequeue.front();
			_sendDequeue.pop_front();

			if (buffer == nullptr || buffer->IsCompleted())
			{
				continue;
			}

			// wsabuf 세팅
			buffer->GetWSABUF(wsabufs[sendCount]);
			sendBuffers.push_back(buffer);
			++sendCount;
		}
	}

	// 보낼 데이터가 없다면 송신 종료
	if (sendCount == 0)
	{
		_sending.store(false);
		return;
	}

	// 비동기 송신 요청
	DWORD sendBytes = 0;
	if (::WSASend(_socket, wsabufs, sendCount, &sendBytes, 0, reinterpret_cast<OVERLAPPED*>(&_sendContext), NULL) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 전송실패 - 모든 버퍼 해제
			int32 errorCode = ::WSAGetLastError();
			LogError("WSASend 실패, 해당 세션 종료", errorCode);
			Close();
			return;
		}
	}
	// send 성공 시 _sendContext.buffers에 저장
	{
		LOCK_GUARD;

		_sendContext.buffers.clear();
		_sendContext.buffers = std::move(sendBuffers);
	}
}

// Recv 완료 & 패킷처리
void Session::OnRecvCompleted(int32 bytesTransferred)
{
	// 수신 데이터가 0일 경우 연결 해제
	if (bytesTransferred == 0)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("Recv Size 0: ", errorCode);
		Close();
		return;
	}
	// 마지막 수신 시간 업데이트
	_lastRecvTime = NOW;
	// 수신 버퍼 위치 이동
	_recvContext.recvBuffer.MoveWritePos(bytesTransferred);
	// 패킷 처리
	while (true)
	{
		// 헤더 크기 이상의 데이터가 있는지 확인
		uint32 packetSize = _recvContext.recvBuffer.GetDataSize();
		if (sizeof(PacketHeader) > packetSize)
		{
			break;
		}
		// 헤더 읽어오기
		const PacketHeader* header = reinterpret_cast<const PacketHeader*>(_recvContext.recvBuffer.GetReadPtr());
		const uint16 dataSize = header->size;

		// 사이즈 검사
		if (dataSize > MAX_PACKET_SIZE || sizeof(PacketHeader) > dataSize)
		{
			LOG_ERROR("잘못된 패킷 크기: " + std::to_string(dataSize));
			Close();
			return;
		}
		
		// 패킷 전체크기만큼 데이터를 수신했는지 확인
		if (dataSize > _recvContext.recvBuffer.GetDataSize())
		{
			break;
		}
		// 패킷 처리
		_service->OnRecv(shared_from_this(), _recvContext.recvBuffer.GetReadPtr(), dataSize);
		_recvContext.recvBuffer.MoveReadPos(dataSize);
	}
	// 버퍼 정리
	_recvContext.recvBuffer.Compact();
	// 다음 수신 요청
	ProcessRecv();
}

// Send 완료 & 남은 데이터 있을시 이어서 전송
void Session::OnSendCompleted(int32 bytesTransferred)
{
	// 마지막 송신 시간 업데이트
	_lastSendTime = NOW;

	// 송신 데이터가 0일 경우 연결 해제
	if (bytesTransferred == 0)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("Send Size 0: ", errorCode);
		Close();
		return;
	}

	// 전체 sendBuffer
	Vector<SendBufferRef> sendBuffers;
	{
		LOCK_GUARD;

		sendBuffers = std::move(_sendContext.buffers);
	}
	
	// 남은 전송 크기
	int32 remainBytes = bytesTransferred;
	// 부분 전송된 버퍼
	Vector<SendBufferRef> remainSendBuffer;
	remainSendBuffer.reserve(MAX_SEND_BUFFER_COUNT);

	for (SendBufferRef& buffer : sendBuffers)
	{
		if (remainBytes <= 0)
		{
			break;
		}
		if (buffer == nullptr)
		{
			continue;
		}

		// 사용크기(데이터 크기)
		int32 useSize = static_cast<int32>(buffer->GetDataSize());
		if (remainBytes >= useSize)
		{
			// 완전 송신
			buffer->OnSendCompleted(useSize);
			remainBytes -= useSize;
		}
		else
		{
			// 부분 송신
			buffer->OnSendCompleted(remainBytes);
			remainBytes = 0;
		}
		// 미전송 버퍼 저장
		if (buffer->IsCompleted() == false)
		{
			remainSendBuffer.push_back(buffer);
		}
	}

	// 큐에 남은 데이터가 있거나 부분 전송된 버퍼가 있으면 이어서 전송
	bool processSend = false;
	{
		LOCK_GUARD;

		// 부분 전송된 버퍼가 있다면, 다음에 전송하기 위해 _sendDequeue 앞쪽에 삽입함
		for (auto it = remainSendBuffer.rbegin(); it != remainSendBuffer.rend(); ++it)
		{
			_sendDequeue.push_front(*it);
		}
		// 남은데이터가 있으면 지역변수 flag 변경
		if (_sendDequeue.empty() == false)
		{
			processSend = true;
		}
		else
		{
			_sending.store(false);
		}
	}

	// 남아있는 데이터 send
	if (processSend)
	{
		ProcessSend();
	}

	// 송신 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnSend(shared_from_this(), bytesTransferred);
}

// 타임아웃 체크
bool Session::IsTimeout(std::chrono::seconds timeout /*= TIMEOUT_SECONDS*/) const
{
	return (NOW - _lastSendTime) > timeout;
}

// 세션 ID 얻기
SessionID Session::GetSessionID() const
{
	return _sessionID;
}

// 소켓 가져오기
SOCKET Session::GetSocket() const
{
	return _socket;
}

// 로그 찍기
// ERROR, WARNING
void Session::LogError(const std::string& msg, const  int32 errorCode, const LogType type /*= LogType::Error*/)
{
	std::string errorMsg = msg;
	errorMsg += " [errorCode: " + std::to_string(errorCode) + "]";

	if (type == LogType::Error)
	{
		LOG_ERROR(errorMsg);
	}
	else if (type == LogType::Warning)
	{
		LOG_WARNING(errorMsg);
	}
}
