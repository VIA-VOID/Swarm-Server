#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\x64Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\x64Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\x64Release\\serverCore.lib")
#pragma comment(lib, "Protobuf\\x64Release\\libprotobuf.lib")
#endif

#include "Pch/CorePch.h"
#include "Protocol/Protocol.pb.h"
#include "Protocol/Enum.pb.h"
#include "Protocol/Struct.pb.h"
#include "Protocol/ProtoOperator.h"
#include "Network/Session.h"
#include "Job/JobGroup.h"

#include "GameDefine.h"
#include "GameMacro.h"
#include "Packet/PacketHandler.h"
#include "World/Zone/Vector3d.h"
