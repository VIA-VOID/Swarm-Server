#include "pch.h"
#include "IocpCore.h"
#include "Session.h"
#include "SocketUtil.h"
#include "Utils/Utils.h"
#include "SessionManager.h"

/*----------------------------
		IocpCore
----------------------------*/

IocpCore::IocpCore()
	: _acceptEx(nullptr), _connectEx(nullptr), _listenSocket(INVALID_SOCKET), _iocpHandle(NULL), _service(nullptr)
{
	_running.store(false, std::memory_order_relaxed);
}

// 종료
void IocpCore::Shutdown()
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

// IocpCore 시작
// Accept & Worker Thread 생성
bool IocpCore::Start(uint16 port)
{
	// 이미 실행 중이면 중단
	if (_running.load())
	{
		return false;
	}
	// 소켓 생성 및 Bind
	if (CreateSocketAndBind(_listenSocket, port) == false)
	{
		return false;
	}
	// Listen
	if (SocketUtil::ListenSocket(_listenSocket) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"Socket Listen 실패", errorCode);
		return false;
	}
	// acceptEx 함수 로딩
	if (WSAIoctlAcceptEx() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"AcceptEx 함수 로딩 실패", errorCode);
		return false;
	}
	// Accept 시작
	for (int32 acceptCount = 0; acceptCount < ACCEPT_NUM; acceptCount++)
	{
		RequestAccept();
	}
	// 워커 스레드 시작
	StartWorkerThreads(MAX_WORKER_THREAD_NUM);

	// HeartBeat 작업 등록
	StartHeartbeatTask();

	LOG_SYSTEM(L"IOCP 서버 시작 (포트: " + std::to_wstring(port) + L")");
	return true;
}

// 서버에 연결
bool IocpCore::Connect(const std::string& address, uint16 port)
{
	// session 생성
	Session* session = SessionMgr.Create();
	// 소켓 생성
	SOCKET clientSocket = INVALID_SOCKET;
	// 소켓 생성 및 Bind
	if (CreateSocketAndBind(clientSocket, 0) == false)
	{
		return false;
	}
	// 세션에 소켓, 서비스 설정
	session->PreInit(clientSocket, _service);
	// connectEx 함수 로딩
	if (WSAIoctlConnectEx(clientSocket) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"ConnectEx 함수 로딩 실패", errorCode);
		return false;
	}
	// 서버 연결 시도
	ProcessConnect(session, address, port);
	
	LOG_SYSTEM(L"IOCP 클라이언트 시작 (서버 주소: " + Utils::ConvertUtf16(address) + L", 포트: " + std::to_wstring(port) + L")");
	return true;
}

// 서비스 연결
void IocpCore::ConnectService(CoreService* service)
{
	_service = service;
}

// 워커 스레드 시작
void IocpCore::StartWorkerThreads(uint16 maxWorkerThreadNum)
{
	_running.store(true);
	// IO 워커 스레드 시작
	ThreadMgr.LaunchGroup(JobGroups::Network, maxWorkerThreadNum, [this]() { IOWorkerThread(); });
}

// 소켓 생성 및 Bind
bool IocpCore::CreateSocketAndBind(SOCKET& socket, uint16 port)
{
	// WSAStartup
	if (SocketUtil::InitWinSocket() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"WSAStartup 실패", errorCode);
		return false;
	}
	// 소켓생성
	socket = SocketUtil::CreateSocket();
	if (socket == INVALID_SOCKET)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"CreateSocket 실패", errorCode);
		return false;
	}
	// Bind
	if (SocketUtil::BindSocket(socket, port) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"Socket Bind 실패", errorCode);
		return false;
	}
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	if (InitIocp(socket) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"CreateIoCompletionPort 실패", errorCode);
		return false;
	}

	return true;
}

// 입출력 완료 포트 새로 생성 & 소켓과 연결
bool IocpCore::InitIocp(SOCKET socket)
{
	// 입출력 완료 포트 새로 생성
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_iocpHandle == NULL)
	{
		return false;
	}
	// 소켓과 입출력 완료 포트를 연결
	if (::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), _iocpHandle, 0, 0) == NULL)
	{
		return false;
	}
	return true;
}

// connectEx 함수 로딩
bool IocpCore::WSAIoctlConnectEx(SOCKET socket)
{
	GUID guidConnectEx = WSAID_CONNECTEX;
	DWORD bytesReturned = 0;

	if (::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidConnectEx, sizeof(guidConnectEx),
		&_connectEx, sizeof(_connectEx),
		&bytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

// connectEx 실행
void IocpCore::ProcessConnect(Session* session, const std::string& address, uint16 port)
{
	ConnectContext* connectContext = session->GetConnectContext();
	ZeroMemory(&connectContext->overlapped, sizeof(connectContext->overlapped));
	connectContext->session = session;

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	::inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);

	// ConnectEx 호출
	DWORD bytesSent = 0;
	if (_connectEx(session->GetSocket(), (SOCKADDR*)&serverAddr, sizeof(serverAddr), 
		nullptr, 0, &bytesSent, (LPOVERLAPPED)connectContext) == FALSE)
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			LogError(L"ConnectEx 실패", errorCode);
			return;
		}
	}
}

// connect 완료
bool IocpCore::OnConnectCompleted(OverlappedEx* overlappedEx)
{
	// ConnectContext 복원
	ConnectContext* connectContext = reinterpret_cast<ConnectContext*>(overlappedEx);
	Session* session = connectContext->session;
	SOCKET clientSocket = session->GetSocket();

	// 소켓 옵션 업데이트 (SO_UPDATE_CONNECT_CONTEXT)
	if (::setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError(L"SO_UPDATE_CONNECT_CONTEXT 설정 실패", errorCode);
		SessionMgr.Release(session);
		return false;
	}

	// 세션 초기화
	if (session->Init(clientSocket, _iocpHandle) == false)
	{
		SessionMgr.Release(session);
		return false;
	}
	// 세션 매니저에 등록
	SessionMgr.Add(session);

	return true;
}

// acceptEx 함수 로딩
bool IocpCore::WSAIoctlAcceptEx()
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
void IocpCore::RequestAccept()
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

	// 세션에 소켓, 서비스 설정
	session->PreInit(clientSocket, _service);
	// AcceptEx 요청
	ProcessAccept(session);
}

// AcceptEx 실행
void IocpCore::ProcessAccept(Session* session)
{
	// AcceptContext 초기화
	AcceptContext* acceptContext = session->GetAcceptContext();
	ZeroMemory(&acceptContext->overlapped, sizeof(acceptContext->overlapped));
	acceptContext->type = NetworkIOType::Accept;
	acceptContext->session = session;

	SOCKET clientSocket = session->GetSocket();
	// AcceptEx 요청
	DWORD bytesReceived = 0;
	if (_acceptEx(_listenSocket, clientSocket, acceptContext->acceptBuffer.data(),
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&bytesReceived, reinterpret_cast<LPOVERLAPPED>(&acceptContext->overlapped)) == FALSE)
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
bool IocpCore::OnAcceptCompleted(OverlappedEx* overlappedEx)
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
		::closesocket(clientSocket);
		SessionMgr.Release(session);
		// 다시 accept 걸어줌
		RequestAccept();
		return false;
	}
	// 세션 초기화
	if (session->Init(clientSocket, _iocpHandle) == false)
	{
		::closesocket(clientSocket);
		// 다시 AcceptEx 요청
		ProcessAccept(session);
		return false;
	}
	// 세션 매니저에 등록
	SessionMgr.Add(session);
	// 다시 accept 걸어줌
	RequestAccept();
	return true;
}

// GQCS 워커 스레드
void IocpCore::IOWorkerThread()
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
		// 타입에 따른 처리
		switch (overlappedEx->type)
		{
		case NetworkIOType::Accept:
			OnAcceptCompleted(overlappedEx);
			break;

		case NetworkIOType::Connect:
			OnConnectCompleted(overlappedEx);
			break;

		case NetworkIOType::Recv:
		case NetworkIOType::Send:
		{
			// 세션 I/O 완료 처리
			Session* session = reinterpret_cast<Session*>(completionKey);
			if (session == nullptr)
			{
				continue;
			}

			// 연결 종료 확인
			if (bytesTransferred == 0 &&
				(overlappedEx->type == NetworkIOType::Recv || overlappedEx->type == NetworkIOType::Send))
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
		break;
		}
	}
}

// 워커 스레드 깨우기
void IocpCore::WakeUpIOWorkerThreads()
{
	// 스레드 개수만큼 종료 신호 전송
	for (int32 count = 0; count < MAX_WORKER_THREAD_NUM; count++)
	{
		::PostQueuedCompletionStatus(_iocpHandle, 0, 0xFFFFFFFF, nullptr);
	}
	LOG_SYSTEM(L"모든 워커 스레드 종료 신호 전송");
}

// 세션 타임아웃 체크 & Job 재등록
void IocpCore::CheckTickTimeout()
{
	// 세션 타임아웃 체크
	SessionMgr.Tick();
	// 다시 10초 후에 실행
	JobQ.DoAsyncAfter(10000, [this]() { CheckTickTimeout(); });
}

// HeartBeat 체크 작업등록
void IocpCore::StartHeartbeatTask()
{
	JobQ.DoAsync([this]() { CheckTickTimeout(); });
}

// 로그 찍기
void IocpCore::LogError(const std::wstring& msg, const int32 errorCode)
{
	std::wstring errorMsg = msg;
	errorMsg += L" [errorCode: " + Utils::ToWString(errorCode) + L"]";
	LOG_ERROR(errorMsg);
}
