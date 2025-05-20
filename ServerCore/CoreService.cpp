#include "pch.h"
#include "CoreService.h"
#include "Network/Session.h"
#include "Network/SessionManager.h"
#include "Network/IocpCore.h"

/*----------------------------
		CoreService
----------------------------*/

CoreService::CoreService(ServiceType type)
	: _iocpCore(nullptr)
{
	_type = type;

	MemoryMgr.Init();

#if _DEBUG
	::SetConsoleOutputCP(CP_UTF8);
	DeadLockMgr.Init();
#endif
	JobGroupMgr.Init();
	ThreadMgr.Init();
	JobQ.Init();
	SessionMgr.Init();
	LogMgr.Init();
	CrashDump::Init();
}

void CoreService::Start(uint16 port, const std::string& address /*= ""*/)
{
	_iocpCore = ObjectPool<IocpCore>::Allocate();
	_iocpCore->ConnectService(this);

	if (_type == ServiceType::Server)
	{
		_iocpCore->Start(port);
	}
	else
	{
		_iocpCore->Connect(address, port);
	}
}

void CoreService::Stop()
{
	if (_iocpCore != nullptr)
	{
		_iocpCore->Shutdown();
		ObjectPool<IocpCore>::Release(_iocpCore);
		_iocpCore = nullptr;
	}	

	// 싱글톤 해제
#if _DEBUG
	DeadLockMgr.Shutdown();
#endif

	LogMgr.Shutdown();
	JobQ.Shutdown();
	SessionMgr.Shutdown();
	JobGroupMgr.Shutdown();
	ThreadMgr.Shutdown();
	MemoryMgr.Shutdown();
}
