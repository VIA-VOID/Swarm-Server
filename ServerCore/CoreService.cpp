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

void CoreService::Run(uint16 port, const std::string& address /*= ""*/, int16 connectCount /*= 1*/)
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

void CoreService::DummyConnect(uint16 port, const std::string& address, int16 connectCount)
{
	// 워커 스레드 생성
	_iocpCore->StartWorkerThreads(MAX_WORKER_THREAD_NUM);

	LOG_SYSTEM(L"Dummy Client 생성 시작");
	
	// BATCH_SIZE 만큼 나누어서 연결
	for (int32 i = 0; i < connectCount; i += BATCH_SIZE)
	{
		int32 batchCount = min(BATCH_SIZE, connectCount - i);

		for (int32 j = 0; j < batchCount; j++)
		{
			_iocpCore->Connect(address, port);
		}

		LOG_SYSTEM(L"Connect: " + std::to_wstring(i) + L" ~ " + std::to_wstring(i + batchCount - 1));

		// 배치 간 간격
		std::this_thread::sleep_for(std::chrono::milliseconds(BATCH_DELAY_MS));
	}
}
