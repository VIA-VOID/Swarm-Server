#include "pch.h"
#include "IocpCore.h"
#include "Session.h"
#include "SocketUtil.h"
#include "SessionManager.h"

/*----------------------------
		IocpCore
----------------------------*/

IocpCore::IocpCore()
	: _acceptEx(nullptr), _connectEx(nullptr), _listenSocket(INVALID_SOCKET), _iocpHandle(INVALID_HANDLE_VALUE), _service(nullptr)
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
		LogError("Socket Listen 실패", errorCode);
		return false;
	}
	// acceptEx 함수 로딩
	if (WSAIoctlAcceptEx() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("AcceptEx 함수 로딩 실패", errorCode);
		return false;
	}
	// Accept 시작
	for (uint32 acceptCount = 0; acceptCount < ACCEPT_NUM; acceptCount++)
	{
		RequestAccept();
	}
	// 워커 스레드 시작
	StartWorkerThreads(MAX_WORKER_THREAD_NUM);

	// HeartBeat 작업 등록
	StartHeartbeatTask();

	LOG_SYSTEM("IOCP 서버 시작 (포트: " + std::to_string(port) + ")");
	return true;
}

// 서버에 연결
bool IocpCore::Connect(const std::string& address, uint16 port, uint16 maxWorkerThreadNum)
{
	// session 생성
	SessionRef session = SessionMgr.Create();
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
		LogError("ConnectEx 함수 로딩 실패", errorCode);
		return false;
	}
	// 워커 스레드 시작
	if (_running.load() == false)
	{
		StartWorkerThreads(maxWorkerThreadNum);
	}
	// 서버 연결 시도
	ProcessConnect(session, address, port);
	
	LOG_SYSTEM("IOCP 클라이언트 시작 (서버 주소: " + address + ", 포트: " + std::to_string(port) + ")");
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
	ThreadMgr.Launch("Network", maxWorkerThreadNum, [this]() { IOWorkerThread(); });
}

// 소켓 생성 및 Bind
bool IocpCore::CreateSocketAndBind(SOCKET& socket, uint16 port)
{
	// WSAStartup
	if (SocketUtil::InitWinSocket() == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("WSAStartup 실패", errorCode);
		return false;
	}
	// 소켓생성
	socket = SocketUtil::CreateSocket();
	if (socket == INVALID_SOCKET)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("CreateSocket 실패", errorCode);
		return false;
	}
	// Bind
	if (SocketUtil::BindSocket(socket, port) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("Socket Bind 실패", errorCode);
		return false;
	}
	// 입출력 완료 포트 새로 생성 & 소켓과 연결
	if (InitIocp(socket) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("CreateIoCompletionPort 실패", errorCode);
		return false;
	}

	return true;
}

// 입출력 완료 포트 새로 생성 & 소켓과 연결
bool IocpCore::InitIocp(SOCKET socket)
{
	// 입출력 완료 포트 새로 생성
	if (_iocpHandle == INVALID_HANDLE_VALUE)
	{
		_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (_iocpHandle == NULL)
		{
			return false;
		}
	}
	// 소켓과 입출력 완료 포트를 연결
	if (_service && _service->IsServerType())
	{
		if (::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), _iocpHandle, 0, 0) == NULL)
		{
			return false;
		}
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
void IocpCore::ProcessConnect(SessionRef session, const std::string& address, uint16 port)
{
	ConnectContext* connectContext = ObjectPool<ConnectContext>::Allocate();
	ZeroMemory(&connectContext->overlapped, sizeof(connectContext->overlapped));
	connectContext->type = NetworkIOType::Connect;
	connectContext->session = session;

	// 소켓과 입출력 완료 포트를 연결
	if (::CreateIoCompletionPort(reinterpret_cast<HANDLE>(session->GetSocket()), _iocpHandle, reinterpret_cast<ULONG_PTR>(session.get()), 0) == NULL)
	{
		LogError("소켓 IOCP 등록 실패", ::WSAGetLastError());
		return;
	}

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
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			LogError("ConnectEx 실패", errorCode);
			ObjectPool<ConnectContext>::Release(connectContext);
			return;
		}
	}
}

// connect 완료
bool IocpCore::OnConnectCompleted(OverlappedEx* overlappedEx)
{
	// ConnectContext 복원
	ConnectContext* connectContext = reinterpret_cast<ConnectContext*>(overlappedEx);
	SessionRef session = connectContext->session;
	if (session == nullptr)
	{
		LOG_WARNING("세션 nullptr");
		return false;
	}
	SOCKET clientSocket = session->GetSocket();

	// 소켓 옵션 업데이트 (SO_UPDATE_CONNECT_CONTEXT)
	if (::setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("SO_UPDATE_CONNECT_CONTEXT 설정 실패", errorCode);
		ObjectPool<ConnectContext>::Release(connectContext);
		return false;
	}

	// 세션 초기화
	if (session->Init(clientSocket, _iocpHandle, false) == false)
	{
		ObjectPool<ConnectContext>::Release(connectContext);
		return false;
	}
	
	// 세션 매니저에 등록
	SessionMgr.Add(session);
	ObjectPool<ConnectContext>::Release(connectContext);
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
	SessionRef session = SessionMgr.Create();
	if (session == nullptr)
	{
		// Accept 재요청
		LOG_WARNING("세션 생성 실패");
		ReRequestAccept();
		return;
	}

	// 클라이언트 소켓 생성
	SOCKET clientSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (clientSocket == INVALID_SOCKET)
	{
		// Accept 재요청
		LogError("클라이언트 소켓 생성 실패", -1);
		ReRequestAccept();
		return;
	}

	// 세션에 소켓, 서비스 설정
	session->PreInit(clientSocket, _service);
	// AcceptEx 요청
	ProcessAccept(session);
}

// Accept 재요청
void IocpCore::ReRequestAccept(AcceptContext* context/* = nullptr*/)
{
	if (context != nullptr)
	{
		ObjectPool<AcceptContext>::Release(context);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	RequestAccept();
}

// AcceptEx 실행
void IocpCore::ProcessAccept(SessionRef session)
{
	// AcceptContext 초기화
	AcceptContext* acceptContext = ObjectPool<AcceptContext>::Allocate();
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
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			LogError("AcceptEx 실패", errorCode);
			::closesocket(clientSocket);
			ReRequestAccept(acceptContext);
			return;
		}
	}
}

// Accept 완료
bool IocpCore::OnAcceptCompleted(OverlappedEx* overlappedEx)
{
	// AcceptContext 복원
	AcceptContext* acceptContext = reinterpret_cast<AcceptContext*>(overlappedEx);
	SessionRef session = acceptContext->session;
	if (session == nullptr)
	{
		LOG_WARNING("세션 nullptr");
		ReRequestAccept(acceptContext);
;		return false;
	}
	SOCKET clientSocket = session->GetSocket();

	// 소켓 옵션 업데이트 (SO_UPDATE_ACCEPT_CONTEXT)
	// _listenSocket의 특성을 clientSocket에 그대로 적용
	if (::setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<char*>(&_listenSocket), sizeof(_listenSocket)) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		LogError("SO_UPDATE_ACCEPT_CONTEXT 설정 실패", errorCode);
		::closesocket(clientSocket);
		ReRequestAccept(acceptContext);
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
	// Accept 재요청
	ReRequestAccept(acceptContext);
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
		// GQCS 예외
		if (ret == FALSE || overlapped == nullptr)
		{
			// 종료신호
			if (completionKey == 0xFFFFFFFF)
			{
				LOG_INFO("WorkerThread 종료 신호 수신");
				break;
			}

			int32 errorCode = ::WSAGetLastError();
			// 예상 가능한 연결 종료
			if (IsExpectedDisConnect(errorCode))
			{
				if (completionKey != 0)
				{
					Session* session = reinterpret_cast<Session*>(completionKey);
					if (session != nullptr)
					{
						uint64 sessionId = session->GetSessionID().GetID();
						LOG_SYSTEM("[SessionID: " + std::to_string(sessionId) + "] 클라이언트 연결 종료");
						session->Close();
					}
				}
				continue;
			}
			// 이외 에러
			if (errorCode != WAIT_TIMEOUT)
			{
				LogError("GQCS Error", errorCode);
			}
			continue;
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
	LOG_SYSTEM("모든 워커 스레드 종료 신호 전송");
}

// 연결 종료
// 예상가능한 상황들
bool IocpCore::IsExpectedDisConnect(int32 errorCode)
{
	switch (errorCode)
	{
	case ERROR_NETNAME_DELETED:		// 64 - 지정된 네트워크 이름을 더 이상 사용할 수 없습니다.
	case ERROR_CONNECTION_ABORTED:	// 1236 - 로컬 시스템에 의해 네트워크 연결이 중단되었습니다.
	case WSAECONNRESET:				// 10054 - 현재 연결은 원격 호스트에 의해 강제로 끊겼습니다.
		return true;
	default:
		return false;
	}
}

// HeartBeat 체크 작업등록
void IocpCore::StartHeartbeatTask()
{
	if (_running.load() == false)
	{
		return;
	}
	// 세션 하트비트
	SessionMgr.Heartbeat(_service);
	// 다시 HEART_BEAT_INTERVAL 후에 실행
	JobQ.DoAsyncAfter(HEART_BEAT_INTERVAL, [this]() { StartHeartbeatTask(); });
}

// 로그 찍기
void IocpCore::LogError(const std::string& msg, const int32 errorCode)
{
	std::string errorMsg = msg;
	errorMsg += " [errorCode: " + std::to_string(errorCode) + "]";
	LOG_ERROR(errorMsg);
}
