#include "pch.h"
#include "PlayerPacketHandler.h"
#include "Dummy/DummyMove.h"
#include "Dummy/DummyChat.h"

void PlayerPacketHandler::Handle_SC_PLAYER_ENTER_GAME(SessionRef session, const Protocol::SC_PLAYER_ENTER_GAME& packet)
{
	Protocol::PosInfo posInfo = packet.objectinfo().posinfo();

	// 무작위 직선 이동 시킴
	DummyMoveRef dummyMove = ObjectPool<DummyMove>::MakeShared();
	dummyMove->DummyRandomMovement(session, posInfo);

	// 무작위 채팅
	DummyChatRef dummyChat = ObjectPool<DummyChat>::MakeShared(packet.objectinfo().name());
	dummyChat->DummyRandomChat(session);
}

void PlayerPacketHandler::Handle_SC_PLAYER_MOVE(SessionRef session, const Protocol::SC_PLAYER_MOVE& packet)
{
}
