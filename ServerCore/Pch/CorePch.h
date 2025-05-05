#pragma once
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Shlwapi.lib")

#include <Windows.h>
#include <iostream>

#include <thread>
#include <mutex>

#include <condition_variable>

#include "Pch/Types.h"
#include "Pch/CoreMacro.h"
#include "Pch/CoreTLS.h"
#include "Utils/Singleton.h"
#include "Container/Container.h"
#include "Utils/CrashDump.h"
#include "Thread/Lock.h"
#include "Thread/LockGuard.h"
#include "Thread/ThreadManager.h"
#include "Log/LogManager.h"
#include "Memory/MemoryManager.h"
#include "Memory/ObjectPool.h"
#include "Job/JobEventQueue.h"
#include "Job/JobGroupManager.h"
#include "Network/NetworkDefine.h"

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include "ServerCoreService.h"
