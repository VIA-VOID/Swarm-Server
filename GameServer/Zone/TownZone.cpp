#include "pch.h"
#include "TownZone.h"
#include "Packet/PacketHandler.h"
#include "Object/GameObject.h"
#include "Object/Player/Player.h"

TownZone::TownZone(const ZoneInfo& zoneInfo)
	: BaseZone(zoneInfo)
{
}

TownZone::~TownZone()
{

}

void TownZone::ZoneUpdateWorkerThread()
{
}

// Object 스폰
void TownZone::ObjectSpawn(GameObjectRef obj)
{
	// 플레이어 스폰
	if (obj->IsPlayer())
	{
		// 플레이어 입장 알림 패킷 전송
		PlayerRef player = std::static_pointer_cast<Player>(obj);
		Protocol::PlayerInfo playerInfo;

		// 초기 스텟 설정
		player->InitStatInfo();
		// Protocol::PlayerInfo 만들기
		player->MakePlayerInfo(playerInfo);
		
		// 입장 패킷 전송
		PacketHandler::SendPacket(player->GetSession(), playerInfo, PacketID::SC_PLAYER_ENTER_GAME);
	}
}

// Object 디스폰
void TownZone::ObjectDespawn(GameObjectRef obj)
{
}
