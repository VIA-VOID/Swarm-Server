#pragma once
#include "PacketHandler.h"

/*--------------------------------------------------------
				ObjectPacketHandler

- 자동생성된 패킷 핸들러 클래스
- Handle_* 함수 구현
--------------------------------------------------------*/

class ObjectPacketHandler : public PacketHandler
{
public:
	// 자동생성
	// 함수 테이블 등록
	void RegisterHandlers() override
	{
		RegisterPacket<Protocol::SC_OBJECT_SPAWN>(PacketID::SC_OBJECT_SPAWN, Handle_SC_OBJECT_SPAWN);
		RegisterPacket<Protocol::SC_OBJECT_DESPAWN>(PacketID::SC_OBJECT_DESPAWN, Handle_SC_OBJECT_DESPAWN);
	}

private:
	// 자동생성
	static void Handle_SC_OBJECT_SPAWN(SessionRef session, const Protocol::SC_OBJECT_SPAWN& packet);
	static void Handle_SC_OBJECT_DESPAWN(SessionRef session, const Protocol::SC_OBJECT_DESPAWN& packet);
};
