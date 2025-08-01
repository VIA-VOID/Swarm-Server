#include "pch.h"
#include "GameServer.h"
#include "World/WorldManager.h"
#include "Object/Stat/StatManager.h"
#include "Object/Player/Player.h"
#include "Utils/Timer.h"

/*----------------------------
		GameServer
----------------------------*/

GameServer::GameServer() 
	: CoreService(ServiceType::Server)
{
	PacketHandler::Init();
	JobGroup::Init();
	WorldMgr.Init();
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
	ObjectPosition allPosition = player->GetAllObjectPosition();
	ObjectId objectId = player->GetObjectId();
	ZoneType zoneType = allPosition.currentZone;
	Vector3d position = allPosition.currentVector;
	GridIndex gridIndex = allPosition.currentGrid;

	// 상호 참조 해제
	player->DetachSession();
	session->DetachPlayer();

	// 유효한 세션을 가진 주변 플레이어들에게 알림
	LeaveGame(objectId, zoneType, position, gridIndex);
}

void GameServer::OnRecv(SessionRef session, BYTE* buffer, int32 len)
{
	PacketHandler::HandlePacket(session, buffer, len);
}

void GameServer::OnSend(SessionRef session, int32 len)
{

}

void GameServer::OnHeartbeat(SessionRef session)
{
	Protocol::SC_SYSTEM_PING pingPkt;
	pingPkt.set_currenttime(Timer::GetNowMsTime());

	PacketHandler::SendPacket(session, pingPkt, PacketID::SC_SYSTEM_PING);
	session->IncreasePingCount();
}

// 게임 종료시 주변 플레이어에게 알림
void GameServer::LeaveGame(const ObjectId objectId, const ZoneType zoneType, const Vector3d& position, const GridIndex& gridIndex)
{
	// 오브젝트 삭제 요청
	WorldMgr.RequestSectorDelete(objectId, zoneType, gridIndex);

	// 디스폰 패킷 생성
	Protocol::SC_OBJECT_DESPAWN despawnPkt;
	despawnPkt.set_objectid(objectId.GetId());

	// 시야 내 근처 플레이어에게 전송
	Vector<GameObjectRef> objs;
	WorldMgr.GetVisibleObjectsInSectors(position, objs);
	
	for (const auto& obj : objs)
	{
		const PlayerRef& player = std::static_pointer_cast<Player>(obj);
		SessionRef playerSession = player->GetSession();
		if (playerSession)
		{
			PacketHandler::SendPacket(playerSession, despawnPkt, PacketID::SC_OBJECT_DESPAWN);
		}
	}
}
