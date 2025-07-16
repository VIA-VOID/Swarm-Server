#include "pch.h"
#include "PlayerPacketHandler.h"
#include "Object/Player/Player.h"

// 플레이어 입장 및 생성
void PlayerPacketHandler::Handle_CS_PLAYER_ENTER_GAME(SessionRef session, const Protocol::CS_PLAYER_ENTER_GAME& packet)
{
	Protocol::PlayerType playerType = packet.playertype();
	
	PlayerRef player = ObjectPool<Player>::MakeShared(session, playerType, packet.name());
	session->SetPlayer(player.get());

	// 입장
	player->EnterGame(ZoneType::Town);
}

// 플레이어 이동 패킷
void PlayerPacketHandler::Handle_CS_PLAYER_MOVE(SessionRef session, const Protocol::CS_PLAYER_MOVE& packet)
{
	Player* player = session->GetPlayer<Player>();
	if (player == nullptr || player->IsValid() == false)
	{
		return;
	}

	// 이동처리
	player->Move(packet);
}
