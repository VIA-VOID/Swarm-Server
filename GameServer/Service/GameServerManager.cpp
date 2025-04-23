#include "pch.h"
#include "GameServerManager.h"

/*----------------------------
	GameServerManager
----------------------------*/

void GameServerManager::Init()
{
	MemoryMgr.Init();

#if _DEBUG
	SetConsoleOutputCP(CP_UTF8);
	DeadLockMgr.Init();
#endif

	ThreadMgr.Init();
	JobQ.Init();
	LogMgr.Init();
	CrashDump::Init();

	LOG_SYSTEM(L"GameServerManager instance initialized");
}

void GameServerManager::Shutdown()
{
	LOG_WARNING(L"Shutdown Server After 2 seconds");

	std::this_thread::sleep_for(std::chrono::seconds(2));

#if _DEBUG
	DeadLockMgr.Shutdown();
#endif

	LogMgr.Shutdown();
	JobQ.Shutdown();
	ThreadMgr.Shutdown();
	MemoryMgr.Shutdown();
}
