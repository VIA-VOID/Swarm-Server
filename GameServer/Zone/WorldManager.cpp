#include "pch.h"
#include "WorldManager.h"
#include "MapDataLoader.h"
#include "TownZone.h"
#include "Object/GameObject.h"
#include "Object/Player/Player.h"
#include "Utils/Utils.h"

void WorldManager::Init()
{
	// 맵데이터 읽기
	std::string dir = "Data/MapData.json";
	if (MapDataLoader::LoadMapData(dir, _mapData) == false)
	{
		CRASH("맵 데이터 읽기 실패");
		return;
	}
	// 그리드 초기화
	_grid.resize(_mapData.gridY);
	for (int32 y = 0; y < _mapData.gridY; y++)
	{
		_grid[y].resize(_mapData.gridX);
	}
	// zone 초기화
	InitZones();
}

void WorldManager::Shutdown()
{
	LOCK_GUARD;
	
	_grid.clear();
	_zones.clear();
}

// 게임 입장
bool WorldManager::EnterGame(GameObjectRef obj)
{
	BaseZone* townZone = GetZone(ZoneType::Town);
	ZonePos zonePos = townZone->GetZoneInfo().worldPos;
	// 스폰 좌표 설정
	// todo: 맵 디자인 완료 후 스폰 zone 별도 생성, 현재는 맵 내 랜덤좌표
	int32 randomX = Utils::GetRandom<int32>(zonePos.minX, zonePos.maxX);
	int32 randomY = Utils::GetRandom<int32>(zonePos.minY, zonePos.maxY);
	int32 worldZ = 180 * POS_REVISE_NUM;
	int32 randomYaw = Utils::GetRandom<int32>(POS_REVISE_NUM, 180 * POS_REVISE_NUM);
	Vector3d spawnVector(randomX, randomY, _mapData.gridSize, worldZ, randomYaw);

	// 그리드셀에 오브젝트 추가
	GridIndex gridIndex;
	spawnVector.MakeGridIndex(gridIndex);

	{
		LOCK_GUARD;

		// 셀 업데이트
		if (AddObjectToCell(obj, gridIndex) == false)
		{
			return false;
		}
	}

	// 위치 업데이트
	obj->SetWorldPosition(spawnVector);
	// 마을에 스폰
	townZone->ObjectSpawn(obj);

	return true;
}

// Zone 초기화
void WorldManager::InitZones()
{
	for (const ZoneInfo& zoneInfo : _mapData.zones)
	{
		ZoneType zoneType = zoneInfo.zoneType;
		switch (zoneType)
		{
		case ZoneType::Town:
			_zones[zoneType] = ObjectPool<TownZone>::MakeUnique(zoneInfo);
			break;
		}
	}
}

// 그리드좌표 유효성 검사
bool WorldManager::IsVaildGridIndex(const GridIndex& gridIndex) const
{
	if (gridIndex.x < 0 || gridIndex.x > _mapData.gridX || gridIndex.y < 0 || gridIndex.y > _mapData.gridY)
	{
		return false;
	}
	return true;
}

// 셀에 오브젝트 추가
bool WorldManager::AddObjectToCell(const GameObjectRef obj, const GridIndex& gridIndex)
{
	// 유효성 검사
	if (IsVaildGridIndex(gridIndex) == false)
	{
		return false;
	}

	// 셀 삽입
	GridCell& cell = _grid[gridIndex.y][gridIndex.x];
	cell.objects.insert(obj);
	cell.isUpdate = true;

	return true;
}

// zone 가져오기
BaseZone* WorldManager::GetZone(const ZoneType zoneType)
{
	auto findit = _zones.find(zoneType);
	if (findit != _zones.end())
	{
		return findit->second.get();
	}
	return nullptr;
}
