#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "Types.h"
#include "CoreMacro.h"
#include "Utils/Singleton.h"

#include <Windows.h>
#include <iostream>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
