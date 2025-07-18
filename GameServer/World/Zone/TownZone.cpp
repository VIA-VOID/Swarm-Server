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
	ZonePos zonePos = _zoneInfo.worldPos;

	float randomX = static_cast<float>(Utils::GetRandom<int32>(zonePos.minX + _spawnOffset, zonePos.maxX - _spawnOffset));
	float randomY = static_cast<float>(Utils::GetRandom<int32>(zonePos.minY + _spawnOffset, zonePos.maxY - _spawnOffset));
	float worldZ = 180.f;
	float randomYaw = Utils::GetRandom<float>(-179.f, 179.f);
	return Vector3d(randomX, randomY, worldZ, randomYaw);
}
