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
	DeadLockMgr.Init();
#endif
	JobGroupMgr.Init();
	ThreadMgr.Init();
	JobQ.Init();
	SessionMgr.Init();
	LogMgr.Init();
	CrashDump::Init();
}

void CoreService::Run(uint16 port, const std::string& address /*= ""*/, uint16 connectCount /*= 1*/)
{
	_iocpCore = ObjectPool<IocpCore>::Allocate();
	_iocpCore->ConnectService(this);

	if (_type == ServiceType::Server)
	{
		_iocpCore->Start(port);
	}
	else
	{
		DummyConnect(port, address, connectCount);
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

bool CoreService::IsServerType() const
{
	return _type == ServiceType::Server;
}

void CoreService::DummyConnect(uint16 port, const std::string& address, uint16 connectCount)
{
	LOG_SYSTEM("Dummy Client 생성 시작");

	for (uint16 i = 0; i < connectCount; i++)
	{
		_iocpCore->Connect(address, port, 5);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}
