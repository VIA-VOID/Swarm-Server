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
		RegisterPacket<Protocol::CS_PLAYER_CREATE>(PacketID::CS_PLAYER_CREATE, Handle_CS_PLAYER_CREATE);
		RegisterPacket<Protocol::CS_PLAYER_MOVE>(PacketID::CS_PLAYER_MOVE, Handle_CS_PLAYER_MOVE);
	}

private:
	// 자동생성
	static void Handle_CS_PLAYER_CREATE(Session* session, Protocol::CS_PLAYER_CREATE& packet);
	static void Handle_CS_PLAYER_MOVE(Session* session, Protocol::CS_PLAYER_MOVE& packet);
};
