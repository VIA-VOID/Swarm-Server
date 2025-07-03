#include "pch.h"
#include "WorldManager.h"
#include "MapDataLoader.h"
#include "TownZone.h"
#include "Object/GameObject.h"
#include "Object/Player/Player.h"
#include "Utils/Utils.h"

// 캐릭터 시야범위
// ex) 캐릭터 좌표가 (50, 50)이면 (0,0) ~ (100, 100)이 보여야함
constexpr int32 playerVisibleRange = static_cast<int32>(PLAYER_VISUAL_RANGE / 2);

// 게임 입장
void WorldManager::Init()
{
	// 맵데이터 읽기
	std::string dir = "Data/MapData.json";
	if (MapDataLoader::LoadMapData(dir, _mapData) == false)
	{
		CRASH("맵 데이터 읽기 실패");
		return;
	}
	// zone 초기화
	InitZones();
	// Zone별 Grid 초기화
	InitZoneGrids();
	// ZoneGrid 업데이트 스레드 생성
	ThreadMgr.LaunchFrame("ZoneGrid", 1, [this]() { ZoneGridUpdateWorkerThread(); });
}

void WorldManager::Shutdown()
{
	LOCK_GUARD;
	
	_zoneGrids.clear();
	_zones.clear();
}

// ZoneGrid 주기적 업데이트
void WorldManager::ZoneGridUpdateWorkerThread()
{
	// todo
}

// Zone별 오브젝트 추가
void WorldManager::AddObjectToZone(GameObjectRef obj, const ZoneType zoneType, const GridIndex& gridIndex)
{
	ZoneGrid* zoneGrid = FindZoneGrid(zoneType);
	if (zoneGrid == nullptr)
	{
		return;
	}
	// 그리드 범위 유효성 검사
	if (zoneGrid->IsValidGridIndex(gridIndex) == false)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*zoneGrid);
		
		// 셀에 오브젝트 추가
		GridCell& cell = zoneGrid->grid[gridIndex.y][gridIndex.x];
		cell.objects.insert({ obj->GetObjectId(), obj });
		cell.isUpdate = true;
	}
}

// 오브젝트 제거
void WorldManager::RemoveObjectToZone(const ObjectId objId, const ZoneType zoneType, const GridIndex& gridIndex)
{
	ZoneGrid* zoneGrid = FindZoneGrid(zoneType);
	if (zoneGrid == nullptr)
	{
		return;
	}
	// 그리드 범위 유효성 검사
	if (zoneGrid->IsValidGridIndex(gridIndex) == false)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*zoneGrid);
		
		// 셀에 오브젝트 제거
		GridCell& cell = zoneGrid->grid[gridIndex.y][gridIndex.x];
		auto it = cell.objects.find(objId);
		if (it != cell.objects.end())
		{
			cell.objects.erase(it);
			cell.isUpdate = true;
			return;
		}
	}
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

// Zone별 Grid 초기화
void WorldManager::InitZoneGrids()
{
	for (const ZoneInfo& zoneInfo : _mapData.zones) 
	{
		_zoneGrids[zoneInfo.zoneType] = ObjectPool<ZoneGrid>::MakeUnique(zoneInfo);
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

// 시야 내의 플레이어의 Id 목록 가져오기
// 단일 Zone 검색
void WorldManager::GetVisiblePlayersInZone(ZoneType zoneType, const Vector3d& position, Vector<PlayerRef>& outPlayers)
{
	ZoneGrid* zoneGrid = FindZoneGrid(zoneType);
	if (zoneGrid == nullptr)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*zoneGrid);
		
		// 시야범위
		GridIndex gridIndex = position.MakeGridIndex(zoneGrid->worldPos);
		int32 startX = max(0, gridIndex.x - playerVisibleRange);
		int32 startY = max(0, gridIndex.y - playerVisibleRange);
		int32 endX = min(static_cast<int32>(zoneGrid->grid[0].size()) - 1, gridIndex.x + playerVisibleRange);
		int32 endY = min(static_cast<int32>(zoneGrid->grid.size()) - 1, gridIndex.y + playerVisibleRange);

		outPlayers.reserve(playerVisibleRange);

		// 플레이어 id 삽입
		for (int32 y = startY; y <= endY; y++) 
		{
			for (int32 x = startX; x <= endX; x++) 
			{
				GridCell& cell = zoneGrid->grid[y][x];
				if (cell.objects.empty()) 
				{
					continue;
				}
				for (auto it = cell.objects.begin(); it != cell.objects.end(); ++it)
				{
					const GameObjectRef& gameObject = it->second;
					if (gameObject->IsPlayer())
					{
						const PlayerRef& player = std::static_pointer_cast<Player>(gameObject);
						if (player->IsWeakValid())
						{
							outPlayers.push_back(player);
						}
					}
				}
			}
		}

	}
}

// 시야 내 플레이어 검색
// 여러 Zone 검색
void WorldManager::GetVisiblePlayersInZones(const Vector3d& position, Vector<PlayerRef>& outPlayers)
{
	Vector<ZoneType> zoneTypes;
	GetVisibleZones(position, zoneTypes);

	// 각 zoneType별 시야내에 속하는 objectId 가져오기
	for (ZoneType zoneType : zoneTypes)
	{
		GetVisiblePlayersInZone(zoneType, position, outPlayers);
	}
}

// 시야 범위에 포함되는 Zone 목록 가져오기
void WorldManager::GetVisibleZones(const Vector3d& position, Vector<ZoneType>& outZoneTypes)
{
	// 현재 위치 zone
	ZoneType currentZone = GetZoneByPosition(position);
	outZoneTypes.push_back(currentZone);

	// 시야 범위내에 다른 zone이 있다면 삽입
	for (const ZoneInfo& zoneInfo : _mapData.zones) 
	{
		if (zoneInfo.zoneType != currentZone && IsInRange(position, zoneInfo.worldPos, _mapData.gridSize))
		{
			outZoneTypes.push_back(zoneInfo.zoneType);
		}
	}
}

// 시야 범위에 Zone이 속하는지 여부
bool WorldManager::IsInRange(const Vector3d& position, const ZonePos& worldPos, int32 gridSize)
{
	int32 playerWorldRange = playerVisibleRange * gridSize * POS_REVISE_NUM;
	int32 playerMinX = position.GetWorldX() - playerWorldRange;
	int32 playerMaxX = position.GetWorldX() + playerWorldRange;
	int32 playerMinY = position.GetWorldY() - playerWorldRange;
	int32 playerMaxY = position.GetWorldY() + playerWorldRange;

	// 모든 축에서 겹치는지 확인
	bool xOverlap = (playerMinX < worldPos.maxX) && (playerMaxX > worldPos.minX);
	bool yOverlap = (playerMinY < worldPos.maxY) && (playerMaxY > worldPos.minY);

	return xOverlap && yOverlap;
}

// 월드 좌표로 Zone 타입 찾기
ZoneType WorldManager::GetZoneByPosition(const Vector3d& position) const
{
	for (const ZoneInfo& zoneInfo : _mapData.zones) 
	{
		const ZonePos& zonePos = zoneInfo.worldPos;

		if (position.GetWorldX() >= zonePos.minX && position.GetWorldX() < zonePos.maxX
			&& position.GetWorldY() >= zonePos.minY && position.GetWorldY() < zonePos.maxY) 
		{
			return zoneInfo.zoneType;
		}
	}
	// 기본값
	return ZoneType::Town;
}

// zone 가져오기
BaseZone* WorldManager::FindZone(const ZoneType zoneType)
{
	auto findit = _zones.find(zoneType);
	if (findit != _zones.end())
	{
		return findit->second.get();
	}
	CRASH("NOT INIT ZONE!!");
	return nullptr;
}

// zoneGrid 가져오기
ZoneGrid* WorldManager::FindZoneGrid(const ZoneType zoneType)
{
	auto findit = _zoneGrids.find(zoneType);
	if (findit != _zoneGrids.end())
	{
		return findit->second.get();
	}
	CRASH("NOT INIT GRID!!");
	return nullptr;
}
