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

	// 해제전 필요 정보 복사
	ObjectId objectId = player->GetObjectId();
	ZoneType zoneType = player->GetCurrentZone();
	Vector3d position = player->GetWorldPosition();
	GridIndex gridIndex = player->GetCurrentGrid();

	// 상호 참조 해제
	player->DetachSession();
	session->DetachPlayer();

	// 유효한 세션을 가진 주변 플레이어들에게 알림
	JobQ.DoAsync([this, objectId, zoneType, position, gridIndex]()
		{
			LeaveGame(objectId, zoneType, position, gridIndex);
		}, JobGroups::Network);
}

void GameServer::OnRecv(SessionRef session, BYTE* buffer, int32 len)
{
	PacketHandler::HandlePacket(session, buffer, len);
}

void GameServer::OnSend(SessionRef session, int32 len)
{

}

// 게임 종료시 주변 플레이어에게 알림
void GameServer::LeaveGame(const ObjectId objectId, const ZoneType zoneType, const Vector3d& position, const GridIndex& gridIndex)
{
	// 그리드에서 제거
	WorldMgr.RemoveObjectToZone(objectId, zoneType, gridIndex);

	// 디스폰 패킷 생성
	Protocol::SC_PLAYER_DESPAWN despawnPkt;
	despawnPkt.set_objectid(objectId.GetId());

	// 시야 내 근처 플레이어에게 전송
	Vector<PlayerRef> players;
	WorldMgr.GetVisiblePlayersInZones(position, players);
	
	for (const auto& player : players)
	{
		SessionRef playerSession = player->GetSession();
		if (playerSession)
		{
			std::cout << "너 여기 들어와?\n";
			PacketHandler::SendPacket(playerSession, despawnPkt, PacketID::SC_PLAYER_DESPAWN);
		}
	}
}
