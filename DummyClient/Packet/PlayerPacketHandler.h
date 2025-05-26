#pragma once
#include "PacketHandler.h"

/*--------------------------------------------------------
				PlayerPacketHandler

- 자동생성된 패킷 핸들러 클래스
- Handle_* 함수 구현
--------------------------------------------------------*/

class PlayerPacketHandler : public PacketHandler
{
public:
	// 자동생성
	// 함수 테이블 등록
	void RegisterHandlers(PacketFunc* handlers) override
	{
		RegisterPacket<Protocol::SC_PLAYER_CREATE>(PacketID::SC_PLAYER_CREATE, Handle_SC_PLAYER_CREATE);
		RegisterPacket<Protocol::SC_PLAYER_MOVE>(PacketID::SC_PLAYER_MOVE, Handle_SC_PLAYER_MOVE);
	}

private:
	// 자동생성
	static void Handle_SC_PLAYER_CREATE(Session* session, Protocol::SC_PLAYER_CREATE& packet);
	static void Handle_SC_PLAYER_MOVE(Session* session, Protocol::SC_PLAYER_MOVE& packet);
};
