#pragma once
#include "PacketHandler.h"

/*--------------------------------------------------------
				SystemPacketHandler

- 자동생성된 패킷 핸들러 클래스
- Handle_* 함수 구현
--------------------------------------------------------*/

class SystemPacketHandler : public PacketHandler
{
public:
	// 자동생성
	// 함수 테이블 등록
	void RegisterHandlers() override
	{
		RegisterPacket<Protocol::SC_SYSTEM_PING>(PacketID::SC_SYSTEM_PING, Handle_SC_SYSTEM_PING);
	}

private:
	// 자동생성
	static void Handle_SC_SYSTEM_PING(SessionRef session, const Protocol::SC_SYSTEM_PING& packet);
};
