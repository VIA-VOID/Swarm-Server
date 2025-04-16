#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Shlwapi.lib")

#include <Windows.h>
#include <iostream>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include <thread>
#include <mutex>

#include <condition_variable>

#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Pch/Types.h"
#include "Pch/CoreMacro.h"
#include "Pch/CoreTLS.h"
#include "Utils/Singleton.h"
#include "Utils/CrashDump.h"
#include "Thread/Lock.h"
#include "Thread/LockGuard.h"
#include "Thread/ThreadManager.h"
#include "Container/EventLockQueue.h"
#include "Log/LogDispatcher.h"