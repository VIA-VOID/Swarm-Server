#include "pch.h"
#include "GameClient.h"
#include "Packet/PacketHandler.h"

GameClient::GameClient()
	: CoreService(ServiceType::Client)
{
	// 패킷 핸들러 초기화
	PacketHandler::Init();
	// JobGroup 초기화
	JobGroup::Init();
}

GameClient::~GameClient()
{

}

void GameClient::OnConnected(SessionRef session)
{
	LOG_SYSTEM("Server Connect!! session: " + std::to_string(session->GetSessionID().GetID()));

	// 입장 패킷 전송
	Protocol::CS_PLAYER_ENTER_GAME EnterGame;
	EnterGame.set_playertype(Protocol::PLAYER_TYPE_WARRIOR);
	EnterGame.set_name("DUMMY-PLAYER-01");
	PacketHandler::SendPacket(session, EnterGame, PacketID::CS_PLAYER_ENTER_GAME);
}

void GameClient::OnDisconnected(SessionRef session)
{
	LOG_INFO("Client OnDisconnected!! session: " + std::to_string(session->GetSessionID().GetID()));
}

void GameClient::OnRecv(SessionRef session, BYTE* buffer, int32 len)
{
	PacketHandler::HandlePacket(session, buffer, len);
}

void GameClient::OnSend(SessionRef session, int32 len)
{
}

void GameClient::OnHeartbeat(SessionRef session)
{
}
