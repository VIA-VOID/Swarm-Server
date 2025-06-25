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
	void RegisterHandlers() override
	{
		RegisterPacket<Protocol::CS_PLAYER_ENTER_GAME>(PacketID::CS_PLAYER_ENTER_GAME, Handle_CS_PLAYER_ENTER_GAME);
		RegisterPacket<Protocol::CS_PLAYER_MOVE>(PacketID::CS_PLAYER_MOVE, Handle_CS_PLAYER_MOVE);
	}

private:
	// 자동생성
	static void Handle_CS_PLAYER_ENTER_GAME(SessionRef session, const Protocol::CS_PLAYER_ENTER_GAME& packet);
	static void Handle_CS_PLAYER_MOVE(SessionRef session, const Protocol::CS_PLAYER_MOVE& packet);
};
