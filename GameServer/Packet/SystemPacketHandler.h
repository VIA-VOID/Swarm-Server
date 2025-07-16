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
		RegisterPacket<Protocol::CS_SYSTEM_PONG>(PacketID::CS_SYSTEM_PONG, Handle_CS_SYSTEM_PONG);
	}

private:
	// 자동생성
	static void Handle_CS_SYSTEM_PONG(SessionRef session, const Protocol::CS_SYSTEM_PONG& packet);
};
