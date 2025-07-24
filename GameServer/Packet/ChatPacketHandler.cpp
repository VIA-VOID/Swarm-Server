#include "pch.h"
#include "ChatPacketHandler.h"
#include "Object/Player/Player.h"

// 채팅 패킷
void ChatPacketHandler::Handle_CS_CHAT_MSG(SessionRef session, const Protocol::CS_CHAT_MSG& packet)
{
	Player* player = session->GetPlayer<Player>();
	if (player == nullptr || player->IsValid() == false)
	{
		return;
	}
	player->ChatMessage(packet);
}
