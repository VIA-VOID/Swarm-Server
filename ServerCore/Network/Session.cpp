#include "pch.h"
#include "Session.h"
#include "SessionManager.h"
#include "Utils/Utils.h"

/*----------------------------
		Session
----------------------------*/

Session::Session()
	: _socket(INVALID_SOCKET), _iocpHandle(INVALID_HANDLE_VALUE), _sessionID(SessionID::Generate()),
	_clientAddress({}), _lastRecvTime(NOW), _connectedTime(NOW), _service(nullptr)
{
	_connectContext.type = NetworkIOType::Connect;
	_acceptContext.type = NetworkIOType::Accept;
	_recvContext.type = NetworkIOType::Recv;
	_sendContext.type = NetworkIOType::Send;

	_sending.store(0, std::memory_order_relaxed);
	_state.store(SessionState::Active, std::memory_order_relaxed);
}

Session::~Session()
{
	// 세션종료
	Close();
}

// 초기화
bool Session::Init(SOCKET socket, HANDLE iocpHandle)
{
	_socket = socket;
	_lastRecvTime = NOW;
	_connectedTime = NOW;
	_iocpHandle = iocpHandle;
	_sending.store(0, std::memory_order_relaxed);
	_state.store(SessionState::Active, std::memory_order_relaxed);

	// IOCP에 등록
	::CreateIoCompletionPort(reinterpret_cast<HANDLE>(_socket), _iocpHandle, reinterpret_cast<ULONG_PTR>(this), 0);

	// 클라이언트 주소 정보 저장
	socklen_t addrLen = sizeof(_clientAddress);
	if (::getpeername(_socket, reinterpret_cast<SOCKADDR*>(&_clientAddress), &addrLen) == SOCKET_ERROR)
	{
		// 주소 저장 실패시 세션 종료
		int32 errorCode = ::WSAGetLastError();
		LogError(L"getpeername 실패", errorCode, LogType::Warning);
		return false;
	}
	// 비동기 수신 시작
	ProcessRecv();
	// 접속 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnConnected(this);

	return true;
}

// 세션 종료
void Session::Close()
{
	// 이미 종료 요청된 세션이면 리턴
	SessionState currentState = _state.load();
	if (currentState == SessionState::CloseRequest || currentState == SessionState::Closed)
	{
		return;
	}
	// 세션 상태를 종료 요청으로 변경
	_state.store(SessionState::CloseRequest);
	_closeRequestTime = NOW;
	// 소켓 닫기
	if (_socket != INVALID_SOCKET)
	{
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
	// 송신 큐 비우기
	{
		LOCK_GUARD;
		while (_sendQueue.empty() == false)
		{
			SendBuffer* buffer = _sendQueue.front();
			_sendQueue.pop();
			// 자원 해제
			if (buffer != nullptr)
			{
				ObjectPool<SendBuffer>::Release(buffer);
			}
		}
	}
	// 연결 종료 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnDisconnected(this);
	// 지연삭제
	SessionMgr.OnSessionClosed(this);
}

// 세션에 소켓, 서비스 설정
void Session::PreInit(SOCKET socket, CoreService* service)
{
	_socket = socket;
	_service = service;
	_lastRecvTime = NOW;
	_connectedTime = NOW;
	_state.store(SessionState::Active, std::memory_order_relaxed);
}

// WSASend 실행전 유효성, 버퍼 할당
void Session::Send(const BYTE* data, int32 len)
{
	// 활성 상태가 아니면 리턴
	if (IsActive() == false)
	{
		return;
	}
	// 유효성 검사
	if (len <= 0 || len > MAX_PACKET_SIZE)
	{
		// 세션종료
		Close();
		return;
	}
	// 버퍼 할당 및 데이터 복사
	SendBuffer* sendBuffer = ObjectPool<SendBuffer>::Allocate(len);
	if (sendBuffer->Write(data, len) == false)
	{
		// 세션종료
		int32 errorCode = ::WSAGetLastError();
		LogError(L"SendBuffer Write 실패, 해당 세션 종료", errorCode);
		ObjectPool<SendBuffer>::Release(sendBuffer);
		Close();
		return;
	}
	// 비동기 송신 시작
	{
		LOCK_GUARD;
		_sendQueue.push(sendBuffer);

		// WSASend 실행
		if (_sending.load() == false)
		{
			_sending.store(true);
			ProcessSend();
		}
	}
}

// WSARecv 실행
void Session::ProcessRecv()
{
	// 활성 상태가 아니면 리턴
	if (IsActive() == false)
	{
		return;
	}
	// 링버퍼의 특성을 활용하여 나눠서 받을수 있으면 받아서 recv한다
	uint32 directWriteSize = _recvContext.recvBuffer.GetDirectRecvSize();
	uint32 wsaBufCount = 0;

	WSABUF wsaBufs[2];
	wsaBufs[wsaBufCount].buf = reinterpret_cast<CHAR*>(_recvContext.recvBuffer.GetRecvPtr());
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
			LogError(L"WSARecv 실패, 해당 세션 종료", errorCode);
			Close();
		}
	}
}

// WSASend 실행
void Session::ProcessSend()
{
	// 활성 상태가 아니면 리턴
	if (IsActive() == false)
	{
		return;
	}
	ZeroMemory(&_sendContext.overlapped, sizeof(_sendContext.overlapped));
	_sendContext.buffers.reserve(MAX_SEND_BUFFER_COUNT);

	// 한 번에 최대 MAX_SEND_BUFFER_COUNT개의 버퍼를 보냄
	WSABUF wsaBufs[MAX_SEND_BUFFER_COUNT * 2];
	uint32 sendCount = 0;
	{
		LOCK_GUARD;
		uint32 maxCount = min(MAX_SEND_BUFFER_COUNT, static_cast<uint32>(_sendQueue.size()));
		for (uint32 count = 0; count < maxCount; count++)
		{
			SendBuffer* buffer = _sendQueue.front();
			if (buffer == nullptr)
			{
				continue;
			}
			// 링버퍼의 특성을 활용하여 나눠서 읽을수 있으면 합쳐서 send한다
			uint32 directReadSize = buffer->GetDirectSendSize();
			if (directReadSize > 0)
			{
				wsaBufs[sendCount].buf = reinterpret_cast<CHAR*>(buffer->GetSendPtr());
				wsaBufs[sendCount].len = directReadSize;
				++sendCount;
			}

			// 읽을수 있는 크기가 남아있다면 앞에서 가져온다.
			uint32 readSize = buffer->GetUseSize() - directReadSize;
			if (readSize > 0)
			{
				wsaBufs[sendCount].buf = reinterpret_cast<CHAR*>(buffer->GetBufferStart());
				wsaBufs[sendCount].len = readSize;
				++sendCount;
			}

			_sendContext.buffers.push_back(buffer);
			_sendQueue.pop();
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
	if (::WSASend(_socket, wsaBufs, sendCount, &sendBytes, 0, reinterpret_cast<OVERLAPPED*>(&_sendContext), NULL) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 전송실패 - 모든 버퍼 해제
			for (SendBuffer* buffer : _sendContext.buffers)
			{
				if (buffer != nullptr)
				{
					ObjectPool<SendBuffer>::Release(buffer);
				}
			}
			int32 errorCode = ::WSAGetLastError();
			LogError(L"WSASend 실패, 해당 세션 종료", errorCode);
			_sending.store(false);
			Close();
			return;
		}
	}
}

// Recv 완료 & 패킷처리
void Session::OnRecvCompleted(int32 bytesTransferred)
{
	// 마지막 수신 시간 업데이트
	_lastRecvTime = NOW;
	// 수신 버퍼 위치 이동
	_recvContext.recvBuffer.CommitRecv(bytesTransferred);
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
		_service->OnRecv(this, buffer.data(), header.size);
	}
	// 수신버퍼 초기화
	_recvContext.recvBuffer.CleanPos();
	// 다음 수신 요청
	ProcessRecv();
}

// Send 완료 & 남은 데이터 있을시 이어서 전송
void Session::OnSendCompleted(int32 bytesTransferred)
{
	// 각 버퍼마다 ReadPos 이동 처리
	int32 transferred = bytesTransferred;
	// 부분 전송된 버퍼
	Vector<SendBuffer*> partialSendArray;

	for (SendBuffer* buffer : _sendContext.buffers)
	{
		if (transferred == 0)
		{
			break;
		}
		if (buffer == nullptr)
		{
			continue;
		}
		int32 useSize = buffer->GetUseSize();
		// 남은 데이터만큼 ReadPos 이동
		if (transferred >= useSize)
		{
			// 완전 전송
			buffer->CommitSend(useSize);
			buffer->CleanPos();
			transferred -= useSize;
			// 모든 데이터가 전송되었을 경우 해제
			if (buffer->GetUseSize() == 0)
			{
				ObjectPool<SendBuffer>::Release(buffer);
			}
		}
		else
		{
			// 부분 전송
			buffer->CommitSend(transferred);
			partialSendArray.push_back(buffer);
			transferred = 0;
		}
	}
	// 송신 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnSend(this, bytesTransferred);

	// 큐에 남은 데이터가 있거나 부분 전송된 버퍼가 있으면 이어서 전송
	{
		LOCK_GUARD;

		// 부분 전송된 버퍼가 있다면, 다음에 전송하기 위해 _sendQueue 앞쪽에 삽입함
		if (partialSendArray.empty() == false)
		{
			// 임시버퍼
			Vector<SendBuffer*> tempBuffer;
			tempBuffer.reserve(_sendQueue.size() + partialSendArray.size());
			// 임시버퍼에 sendQueue에 담겨있는 데이터를 빼낸다.
			while (_sendQueue.empty() == false)
			{
				tempBuffer.push_back(_sendQueue.front());
				_sendQueue.pop();
			}
			// 순서대로 다시 조립한다.
			// 부분 전송된 버퍼 + 임시버퍼(sendQueue에 원래 있던 데이터)
			for (auto it = partialSendArray.rbegin(); it != partialSendArray.rend(); ++it)
			{
				_sendQueue.push(*it);
			}
			for (SendBuffer* buffer : tempBuffer)
			{
				_sendQueue.push(buffer);
			}
		}

		if (_sendQueue.empty() == false)
		{
			_sending.store(true);
			// 남아있는 데이터 send
			ProcessSend();
		}
		else
		{
			_sending.store(false);
		}
	}
}

// 타임아웃 체크
bool Session::IsTimeout(std::chrono::seconds timeout /*= TIMEOUT_SECONDS*/)
{
	return IsActive() && (NOW - _lastRecvTime) > timeout;
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

// 세션 상태
SessionState Session::GetState()
{
	return _state.load();
}

// ConnectContext 가져오기
ConnectContext* Session::GetConnectContext()
{
	return &_connectContext;
}

// AcceptContext 가져오기
AcceptContext* Session::GetAcceptContext()
{
	return &_acceptContext;
}

// 세션 상태 변경
void Session::SetState(SessionState state)
{
	// 세션 상태 변경 이후 시간 업데이트
	SessionState prevState = _state.exchange(state);
	if (prevState != state && state == SessionState::CloseRequest)
	{
		_closeRequestTime = NOW;
	}
}

// 활성상태 여부
bool Session::IsActive()
{
	return _state.load() == SessionState::Active;
}

// 로그 찍기
// ERROR, WARNING
void Session::LogError(const std::wstring& msg, const  int32 errorCode, const LogType type /*= LogType::Error*/)
{
	std::wstring errorMsg = msg;
	errorMsg += L" [errorCode: " + Utils::ToWString(errorCode) + L"]";

	if (type == LogType::Error)
	{
		LOG_ERROR(errorMsg);
	}
	else if (type == LogType::Warning)
	{
		LOG_WARNING(errorMsg);
	}
}
