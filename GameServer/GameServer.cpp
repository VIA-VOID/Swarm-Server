#include "pch.h"
#include "GameServer.h"
#include "Zone/WorldManager.h"
#include "Object/Stat/StatManager.h"
#include "Object/Player/Player.h"

/*----------------------------
		GameServer
----------------------------*/

GameServer::GameServer() 
	: CoreService(ServiceType::Server)
{
	// 패킷 핸들러 초기화
	PacketHandler::Init();
	// JobGroup 초기화
	JobGroup::Init();
	// WorldManager 초기화
	WorldMgr.Init();
	// StatManager 초기화
	StatMgr.Init();
}

GameServer::~GameServer()
{
	WorldMgr.Shutdown();
	StatMgr.Shutdown();
}

void GameServer::OnConnected(SessionRef session)
{
	LOG_INFO("Client Connect!! session: " + std::to_string(session->GetSessionID().GetID()));
}

void GameServer::OnDisconnected(SessionRef session)
{
	LOG_INFO("Client OnDisconnected!! session: " + std::to_string(session->GetSessionID().GetID()));

	Player* player = session->GetPlayer<Player>();
	if (player == nullptr)
	{
		return;
	}

	// 상호 참조 해제 전 despawn 패킷 전송
	player->LeaveGame();
	// 상호 참조 해제
	player->DetachSession();
	session->DetachPlayer();
}

void GameServer::OnRecv(SessionRef session, BYTE* buffer, int32 len)
{
	PacketHandler::HandlePacket(session, buffer, len);
}

void GameServer::OnSend(SessionRef session, int32 len)
{

}
