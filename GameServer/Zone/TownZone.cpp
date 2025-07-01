#include "pch.h"
#include "TownZone.h"
#include "Utils/Utils.h"
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

void TownZone::ZoneUpdate()
{
}

// 랜덤 스폰 위치 가져오기
Vector3d TownZone::GetRandomSpawnPosition()
{
	// 스폰 좌표 설정
	// todo: 맵 디자인 완료 후 스폰 zone 별도 생성, 현재는 맵 내 랜덤좌표
	ZonePos zonePos = _zoneInfo.worldPos;

	int32 randomX = Utils::GetRandom<int32>(zonePos.minX, zonePos.maxX);
	int32 randomY = Utils::GetRandom<int32>(zonePos.minY, zonePos.maxY);
	int32 worldZ = 180 * POS_REVISE_NUM;
	int32 randomYaw = Utils::GetRandom<int32>(POS_REVISE_NUM, 180 * POS_REVISE_NUM);
	return Vector3d(randomX, randomY, worldZ, randomYaw, _zoneInfo.gridSize);
}
