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
	if (_isClosed.load())
	{
		return;
	}
	// 자원 해제
	CloseResource();
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
	if (_isClosed.load())
	{
		return;
	}
	// flag 변경
	_isClosed.store(true);
	_sending.store(false);
	// 연결 종료 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnDisconnected(shared_from_this());
	// 지연삭제
	SessionMgr.OnSessionClosed(shared_from_this());
	// 자원 해제
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

// WSASend 실행전 유효성, 버퍼 할당
void Session::Send(const SendBufferRef& sendBuffer, int32 len)
{
	// 유효성 검사
	if (len <= 0 || len > MAX_PACKET_SIZE)
	{
		// 세션종료
		Close();
		return;
	}
	// 비동기 송신 시작
	{
		LOCK_GUARD;
		_sendDequeue.push_back(sendBuffer);

		// WSASend 실행
		if (_sending.load() == false)
		{
			_sending.store(true);
			ProcessSend();
		}
	}
}

// 유저 클래스 포인터 참조 정리
void Session::DetachPlayer()
{
	_playerClass = nullptr;
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
		Deque<SendBufferRef> empty;
		std::swap(_sendDequeue, empty);
	}
}

// WSARecv 실행
void Session::ProcessRecv()
{
	// 링버퍼의 특성을 활용하여 나눠서 받을수 있으면 받아서 recv한다
	uint32 directWriteSize = _recvContext.recvBuffer.GetDirectEnqueSize();
	uint32 wsaBufCount = 0;

	WSABUF wsaBufs[2];
	wsaBufs[wsaBufCount].buf = reinterpret_cast<CHAR*>(_recvContext.recvBuffer.GetWritePtr());
	wsaBufs[wsaBufCount].len = directWriteSize;
	++wsaBufCount;

	// 쓸수있는 크기가 남아있다면 앞에서 채운다
	uint32 writeSize = _recvContext.recvBuffer.GetFreeSize() - directWriteSize;
	if (writeSize > 0)
	{
		wsaBufs[wsaBufCount].buf = reinterpret_cast<CHAR*>(_recvContext.recvBuffer.GetBufferStart());
		wsaBufs[wsaBufCount].len = writeSize;
		++wsaBufCount;
	}

	DWORD recvBytes = 0;
	DWORD flags = 0;
	if (::WSARecv(_socket, wsaBufs, wsaBufCount, &recvBytes, &flags, reinterpret_cast<OVERLAPPED*>(&_recvContext), NULL) == SOCKET_ERROR)
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
	ZeroMemory(&_sendContext.overlapped, sizeof(_sendContext.overlapped));
	_sendContext.buffers.reserve(MAX_SEND_BUFFER_COUNT);

	// 한 번에 최대 MAX_SEND_BUFFER_COUNT개의 버퍼를 보냄
	WSABUF wsaBufs[MAX_SEND_BUFFER_COUNT * 2];
	uint32 sendCount = 0;
	uint32 maxCount = min(MAX_SEND_BUFFER_COUNT, static_cast<uint32>(_sendDequeue.size()));
	for (uint32 count = 0; count < maxCount; count++)
	{
		SendBufferRef buffer = _sendDequeue.front();
		if (buffer == nullptr)
		{
			continue;
		}
		
		wsaBufs[sendCount].buf = reinterpret_cast<CHAR*>(buffer->GetReadPtr());
		wsaBufs[sendCount].len = buffer->GetUseSize();
		++sendCount;

		_sendContext.buffers.push_back(buffer);
		_sendDequeue.pop_front();
	}

	// 보낼 데이터가 없다면 송신 종료
	if (sendCount == 0)
	{
		_sending.store(false);
		return;
	}

	// 비동기 송신 요청
	DWORD sendBytes = 0;
	if (::WSASend(_socket, wsaBufs, sendCount, &sendBytes, 0, reinterpret_cast<OVERLAPPED*>(&_sendContext), NULL) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 전송실패 - 모든 버퍼 해제
			int32 errorCode = ::WSAGetLastError();
			LogError("WSASend 실패, 해당 세션 종료", errorCode);
			_sendContext.buffers.clear();
			Close();
			return;
		}
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
		uint32 packetSize = _recvContext.recvBuffer.GetUseSize();
		if (sizeof(PacketHeader) > packetSize)
		{
			break;
		}
		// 헤더 읽어오기
		PacketHeader header;
		_recvContext.recvBuffer.Peek(reinterpret_cast<BYTE*>(&header), sizeof(header), sizeof(header));
		// 패킷 전체크기만큼 데이터를 수신했는지 확인
		if (header.size > packetSize)
		{
			break;
		}
		// 패킷 전체 데이터 읽어오기 + buffer Pos 이동
		Vector<BYTE> buffer(header.size);
		_recvContext.recvBuffer.Read(buffer.data(), header.size, header.size);
		// 컨텐츠 로직에서 구현
		_service->OnRecv(shared_from_this(), buffer.data(), header.size);
	}
	// 수신버퍼 초기화
	_recvContext.recvBuffer.CleanPos();
	// 다음 수신 요청
	ProcessRecv();
}

// Send 완료 & 남은 데이터 있을시 이어서 전송
void Session::OnSendCompleted(int32 bytesTransferred)
{
	// 송신 데이터가 0일 경우 연결 해제
	if (bytesTransferred == 0)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("Send Size 0: ", errorCode);
		_sendContext.buffers.clear();
		Close();
		return;
	}
	// 마지막 송신 시간 업데이트
	_lastSendTime = NOW;
	
	// 전송된 크기
	int32 transferred = bytesTransferred;
	
	// 부분 전송된 버퍼
	Vector<SendBufferRef> remainSendArray;
	remainSendArray.reserve(MAX_SEND_BUFFER_COUNT);

	for (SendBufferRef& buffer : _sendContext.buffers)
	{
		// 사용크기(데이터 크기)
		int32 useSize = buffer->GetUseSize();
		
		// 완전전송됨
		if (transferred >= useSize)
		{
			transferred -= useSize;
			continue;
		}
		else
		{
			// 부분전송됨
			buffer->MoveReadPos(transferred);
			remainSendArray.push_back(buffer);
			transferred = 0;
		}
	}
	// 버퍼 초기화
	_sendContext.buffers.clear();

	// 송신 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnSend(shared_from_this(), bytesTransferred);

	// 큐에 남은 데이터가 있거나 부분 전송된 버퍼가 있으면 이어서 전송
	{
		LOCK_GUARD;

		// 부분 전송된 버퍼가 있다면, 다음에 전송하기 위해 _sendDequeue 앞쪽에 삽입함
		if (remainSendArray.empty() == false)
		{
			for (auto it = remainSendArray.rbegin(); it != remainSendArray.rend(); ++it)
			{
				_sendDequeue.push_front(*it);
			}
			remainSendArray.clear();
		}
		
		// 남아있는 데이터 send
		if (_sendDequeue.empty() == false)
		{
			_sending.store(true);
			ProcessSend();
		}
	}
	_sending.store(false);
}

// 타임아웃 체크
bool Session::IsTimeout(std::chrono::seconds timeout /*= TIMEOUT_SECONDS*/)
{
	return (NOW - _lastSendTime) > timeout;
}

// 세션 ID 얻기
SessionID Session::GetSessionID()
{
	return _sessionID;
}

// 소켓 가져오기
SOCKET Session::GetSocket()
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
