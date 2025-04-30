#include "pch.h"
#include "Session.h"
#include "SessionManager.h"
#include "Utils/Utils.h"

/*----------------------------
		Session
----------------------------*/

Session::Session()
	: _socket(INVALID_SOCKET), _iocpHandle(INVALID_HANDLE_VALUE), _recvBuffer(BUFFER_SIZE), _sessionID(SessionID::Generate()),
	_clientAddress({}), _lastRecvTime(NOW), _connectedTime(NOW), _service(nullptr)
{
	_recvOverlappedEx.type = NetworkIOType::Recv;
	_sendOverlappedEx.type = NetworkIOType::Send;
	_sending.store(0, std::memory_order_relaxed);
	_state.store(SessionState::Active, std::memory_order_relaxed);
}

Session::~Session()
{
	// 세션종료
	Close();
}

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

// 소켓만 먼저 설정
void Session::PreAccept(SOCKET socket, ServerCoreService* service)
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
	uint32 directWriteSize = _recvBuffer.GetDirectRecvSize();
	uint32 wsaBufCount = 0;

	WSABUF wsaBufs[2];
	wsaBufs[wsaBufCount].buf = reinterpret_cast<CHAR*>(_recvBuffer.GetRecvPtr());
	wsaBufs[wsaBufCount].len = directWriteSize;
	++wsaBufCount;

	// 쓸수있는 크기가 남아있다면 앞에서 채운다
	uint32 writeSize = _recvBuffer.GetFreeSize() - directWriteSize;
	if (writeSize > 0)
	{
		wsaBufs[wsaBufCount].buf = reinterpret_cast<CHAR*>(_recvBuffer.GetBufferStart());
		wsaBufs[wsaBufCount].len = writeSize;
		++wsaBufCount;
	}

	DWORD recvBytes = 0;
	DWORD flags = 0;
	if (::WSARecv(_socket, wsaBufs, wsaBufCount, &recvBytes, &flags, reinterpret_cast<OVERLAPPED*>(&_recvOverlappedEx), NULL) == SOCKET_ERROR)
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
	SendContext* sendContext = ObjectPool<SendContext>::Allocate();
	ZeroMemory(&sendContext->overlappedEx, sizeof(sendContext->overlappedEx));
	sendContext->overlappedEx.type = NetworkIOType::Send;
	sendContext->buffers.reserve(MAX_SEND_BUFFER_COUNT);

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

			sendContext->buffers.push_back(buffer);
			_sendQueue.pop();
		}
	}

	// 보낼 데이터가 없다면 송신 종료
	if (sendCount == 0)
	{
		ObjectPool<SendContext>::Release(sendContext);
		_sending.store(false);
		return;
	}

	// 비동기 송신 요청
	DWORD sendBytes = 0;
	if (::WSASend(_socket, wsaBufs, sendCount, &sendBytes, 0, reinterpret_cast<OVERLAPPED*>(&sendContext->overlappedEx), NULL) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 전송실패 - 모든 버퍼 해제
			for (SendBuffer* buffer : sendContext->buffers)
			{
				if (buffer != nullptr)
				{
					ObjectPool<SendBuffer>::Release(buffer);
				}
			}
			ObjectPool<SendContext>::Release(sendContext);
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
	_recvBuffer.CommitRecv(bytesTransferred);
	_recvBuffer.CleanPos();

	// TODO: 패킷처리 로직
	/*
	while (true)
	{
		// OnRecv();
	}
	*/
	// 다음 수신 요청
	ProcessRecv();
}

// Send 완료 & 남은 데이터 있을시 이어서 전송
void Session::OnSendCompleted(int32 bytesTransferred, OverlappedEx* overlappedEx)
{
	// SendContext 복원
	SendContext* sendContext = reinterpret_cast<SendContext*>(overlappedEx);
	// 각 버퍼마다 ReadPos 이동 처리
	int32 transferred = bytesTransferred;
	for (SendBuffer* buffer : sendContext->buffers)
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
		}
		else
		{
			// 부분 전송
			buffer->CommitSend(transferred);
			transferred = 0;
		}

		// 모든 데이터가 전송되었을 경우 해제
		if (buffer->GetUseSize() == 0)
		{
			ObjectPool<SendBuffer>::Release(buffer);
		}
	}
	// 송신 이벤트 호출
	// 컨텐츠 로직에서 구현
	_service->OnSend(this, bytesTransferred);
	// 컨텍스트 정리
	ObjectPool<SendContext>::Release(sendContext);
	// 큐에 남은 데이터가 있으면 이어서 전송
	{
		LOCK_GUARD;
		if (_sendQueue.empty())
		{
			_sending.store(false);
		}
		else
		{
			ProcessSend();
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
