#include "pch.h"
#include "IocpServer.h"
#include "Session.h"
#include "SocketUtil.h"
#include "Utils/Utils.h"
#include "SessionManager.h"

/*----------------------------
		IocpServer
----------------------------*/

IocpServer::IocpServer()
	: _acceptEx(nullptr), _listenSocket(INVALID_SOCKET), _iocpHandle(NULL), _service(nullptr)
{
	_acceptEx = nullptr;
	_running.store(false, std::memory_order_relaxed);
	_listenSocket = INVALID_SOCKET;
	_iocpHandle = NULL;
}

// 종료
void IocpServer::Shutdown()
{
	// 이미 중지되었으면 리턴
	if (_running.exchange(false) == false)
	{
		return;
	}
	// 워커 스레드 깨우기
	WakeUpIOWorkerThreads();

	// 리슨 소켓 닫기
	if (_listenSocket != INVALID_SOCKET)
	{
		::closesocket(_listenSocket);
		_listenSocket = INVALID_SOCKET;
	}
	// IOCP 핸들 닫기
	if (_iocpHandle != NULL)
	{
		::CloseHandle(_iocpHandle);
		_iocpHandle = NULL;
	}
	::WSACleanup();
}

// IocpServer 시작
// Accept & Worker Thread 생성
bool IocpServer::Start(uint16 port)
{
	// 이미 실행 중이면 중단
	if (_running.load())
	{
		return false;
	}
	// WSAStartup
	if (SocketUtil::InitWinSocket() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"WSAStartup 실패", errorCode);
		return false;
	}
	// 소켓생성
	_listenSocket = SocketUtil::CreateSocket();
	if (_listenSocket == INVALID_SOCKET)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"CreateSocket 실패", errorCode);
		return false;
	}
	// WSAStartup
	if (SocketUtil::InitWinSocket() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"WSAStartup 실패", errorCode);
		return false;
	}
	// Bind & Listen
	if (SocketUtil::BindAndListenSocket(_listenSocket, port) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"Socket BindListen 실패", errorCode);
		return false;
	}
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	if (InitIocp() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"CreateIoCompletionPort 실패", errorCode);
		return false;
	}
	// acceptEx 함수 로딩
	if (WSAIoctlAcceptEx() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"AcceptEx 함수 로딩 실패", errorCode);
		return false;
	}

	// 스레드 시작
	_running.store(true);
	// Accept 시작
	for (int32 acceptCount = 0; acceptCount < ACCEPT_NUM; acceptCount++)
	{
		RequestAccept();
	}
	// IO 워커 스레드 시작
	ThreadMgr.LaunchGroup(JobGroups::Network, MAX_WORKER_THREAD_NUM, [this]() { IOWorkerThread(); });
	// HeartBeat 작업 등록
	StartHeartbeatTask();

	LOG_SYSTEM(L"IOCP 서버 시작 (포트: " + std::to_wstring(port) + L")");
	return true;
}

// 서비스 연결
void IocpServer::ConnectService(ServerCoreService* service)
{
	_service = service;
}

// 입출력 완료 포트 새로 생성 & 소켓과 연결
bool IocpServer::InitIocp()
{
	// 입출력 완료 포트 새로 생성
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_iocpHandle == NULL)
	{
		return false;
	}
	// 소켓과 입출력 완료 포트를 연결
	if (::CreateIoCompletionPort(reinterpret_cast<HANDLE>(_listenSocket), _iocpHandle, 0, 0) == NULL)
	{
		return false;
	}
	return true;
}

// acceptEx 함수 로딩
bool IocpServer::WSAIoctlAcceptEx()
{
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytesReturned = 0;

	if (::WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx),
		&_acceptEx, sizeof(_acceptEx),
		&bytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

// Accept 요청
void IocpServer::RequestAccept()
{
	// 세션 생성
	Session* session = SessionMgr.Create();
	if (session == nullptr)
	{
		// 다시 accept 걸어줌
		LOG_WARNING(L"세션 생성 실패");
		JobQ.DoAsyncAfter(100, [this]() { RequestAccept(); }, JobGroups::Network);
		return;
	}

	// 클라이언트 소켓 생성
	SOCKET clientSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (clientSocket == INVALID_SOCKET)
	{
		// 다시 accept 걸어줌
		LogError(L"클라이언트 소켓 생성 실패", -1);
		SessionMgr.Release(session);
		JobQ.DoAsyncAfter(100, [this]() { RequestAccept(); }, JobGroups::Network);
		return;
	}

	// Accept전 세션에 소켓, 서비스 설정
	session->PreAccept(clientSocket, _service);
	// AcceptEx 요청
	ProcessAccept(session);
}

// AcceptEx 실행
void IocpServer::ProcessAccept(Session* session)
{
	// AcceptContext 생성 및 초기화
	AcceptContext acceptContext = session->GetAcceptContext();
	ZeroMemory(&acceptContext.overlapped, sizeof(acceptContext.overlapped));
	acceptContext.type = NetworkIOType::Accept;
	acceptContext.session = session;

	SOCKET clientSocket = session->GetSocket();
	// AcceptEx 요청
	DWORD bytesReceived = 0;
	if (_acceptEx(_listenSocket, clientSocket, acceptContext.acceptBuffer.data(),
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&bytesReceived, reinterpret_cast<LPOVERLAPPED>(&acceptContext.overlapped)) == FALSE)
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			LogError(L"AcceptEx 실패", errorCode);
			::closesocket(clientSocket);
			SessionMgr.Release(session);
			// 다시 accept 걸어줌
			JobQ.DoAsyncAfter(100, [this]() { RequestAccept(); }, JobGroups::Network);
			return;
		}
	}
}

// Accept 완료
bool IocpServer::OnAcceptCompleted(OverlappedEx* overlappedEx)
{
	// AcceptContext 복원
	AcceptContext* acceptContext = reinterpret_cast<AcceptContext*>(overlappedEx);
	Session* session = acceptContext->session;
	SOCKET clientSocket = session->GetSocket();

	// 소켓 옵션 업데이트 (SO_UPDATE_ACCEPT_CONTEXT)
	// _listenSocket의 특성을 clientSocket에 그대로 적용
	if (::setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<char*>(&_listenSocket), sizeof(_listenSocket)) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"SO_UPDATE_ACCEPT_CONTEXT 설정 실패", errorCode);
		ObjectPool<AcceptContext>::Release(acceptContext);
		::closesocket(clientSocket);
		SessionMgr.Release(session);
		// 다시 accept 걸어줌
		RequestAccept();
		return false;
	}
	// 세션 초기화
	if (session->Init(clientSocket, _iocpHandle) == false)
	{
		// 초기화 실패
		ObjectPool<AcceptContext>::Release(acceptContext);
		::closesocket(clientSocket);
		// 다시 AcceptEx 요청
		ProcessAccept(session);
		return false;
	}
	// 세션 매니저에 등록
	SessionMgr.Add(session);
	// AcceptContext 해제
	ObjectPool<AcceptContext>::Release(acceptContext);
	// 다시 accept 걸어줌
	RequestAccept();
	return true;
}

// GQCS 워커 스레드
void IocpServer::IOWorkerThread()
{
	while (_running)
	{
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;

		// 완료된 I/O 작업 대기
		BOOL ret = ::GetQueuedCompletionStatus(_iocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE);
		if (ret == FALSE || overlapped == nullptr)
		{
			// 종료신호
			if (completionKey == 0xFFFFFFFF)
			{
				LOG_INFO(L"WorkerThread 종료 신호 수신");
				break;
			}

			int32 errorCode = ::WSAGetLastError();
			if (errorCode != WAIT_TIMEOUT)
			{
				LogError(L"GQCS Error", errorCode);
				continue;
			}
		}
		// OverlappedEx 복원
		OverlappedEx* overlappedEx = reinterpret_cast<OverlappedEx*>(overlapped);
		// Accept 완료 처리
		if (overlappedEx->type == NetworkIOType::Accept)
		{
			if (OnAcceptCompleted(overlappedEx) == false)
			{
				continue;
			}
		}

		// 세션 I/O 완료 처리
		Session* session = reinterpret_cast<Session*>(completionKey);
		if (session == nullptr)
		{
			continue;
		}
		// 연결 종료 확인
		if (bytesTransferred == 0 &&
			(overlappedEx->type == NetworkIOType::Recv) || (overlappedEx->type == NetworkIOType::Send))
		{
			session->Close();
			continue;
		}
		// I/O 타입에 따른 처리
		switch (overlappedEx->type)
		{
		case NetworkIOType::Recv:
			session->OnRecvCompleted(bytesTransferred);
			break;
		case NetworkIOType::Send:
			session->OnSendCompleted(bytesTransferred);
			break;
		}
	}
}

// 워커 스레드 깨우기
void IocpServer::WakeUpIOWorkerThreads()
{
	// 스레드 개수만큼 종료 신호 전송
	for (int32 count = 0; count < MAX_WORKER_THREAD_NUM; count++)
	{
		::PostQueuedCompletionStatus(_iocpHandle, 0, 0xFFFFFFFF, nullptr);
	}
	LOG_SYSTEM(L"모든 워커 스레드 종료 신호 전송");
}

// 세션 타임아웃 체크 & Job 재등록
void IocpServer::CheckTickTimeout()
{
	// 세션 타임아웃 체크
	SessionMgr.Tick();
	// 다시 10초 후에 실행
	JobQ.DoAsyncAfter(10000, [this]() { CheckTickTimeout(); });
}

// HeartBeat 체크 작업등록
void IocpServer::StartHeartbeatTask()
{
	JobQ.DoAsync([this]() { CheckTickTimeout(); });
}

// 로그 찍기
void IocpServer::LogError(const std::wstring& msg, const int32 errorCode)
{
	std::wstring errorMsg = msg;
	errorMsg += L" [errorCode: " + Utils::ToWString(errorCode) + L"]";
	LOG_ERROR(errorMsg);
}
