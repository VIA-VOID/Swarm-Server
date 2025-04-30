#include "pch.h"
#include "ServerCoreService.h"
#include "Network/Session.h"
#include "Network/SessionManager.h"
#include "Network/IocpServer.h"

/*----------------------------
		NetworkService
----------------------------*/

NetworkService::NetworkService()
{
	MemoryMgr.Init();

#if _DEBUG
	::SetConsoleOutputCP(CP_UTF8);
	DeadLockMgr.Init();
#endif
	ThreadMgr.Init();
	JobQ.Init();
	SessionMgr.Init();
	LogMgr.Init();
	CrashDump::Init();
}

/*----------------------------
		ServerCoreService
----------------------------*/

ServerCoreService::ServerCoreService()
	: _iocpServer(nullptr)
{
}

// iocpServer와 연결 및 시작 
void ServerCoreService::StartServer(uint16 port)
{
	_iocpServer = ObjectPool<IocpServer>::Allocate();
	_iocpServer->ConnectService(this);
	_iocpServer->Start(port);
}

// 자원 해제
void ServerCoreService::CloseServer()
{
	LOG_WARNING(L"Shutdown Server After 3 seconds");
	std::this_thread::sleep_for(std::chrono::seconds(3));

	_iocpServer->Shutdown();
	ObjectPool<IocpServer>::Release(_iocpServer);

#if _DEBUG
	DeadLockMgr.Shutdown();
#endif

	LogMgr.Shutdown();
	JobQ.Shutdown();
	SessionMgr.Shutdown();
	ThreadMgr.Shutdown();
	MemoryMgr.Shutdown();
}
