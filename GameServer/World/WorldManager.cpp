#include "pch.h"
#include "WorldManager.h"
#include "MapDataLoader.h"
#include "Zone/TownZone.h"
#include "Zone/PveZone.h"
#include "Zone/PvpZone.h"
#include "Zone/BossZone.h"
#include "Object/GameObject.h"
#include "Utils/Utils.h"
#include "Network/SessionManager.h"

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
	// Sector 초기화
	InitSectors();
	// ZoneGrid 업데이트 스레드 생성
	ThreadMgr.LaunchFrame("WorldZone", 1, [this]() { SectorUpdateWorkerThread(); });
}

void WorldManager::Shutdown()
{
	LOCK_GUARD;

	_forceDeleteObjs.clear();
	_sectorUpdateSet.clear();
	_zoneSectors.clear();
	_zones.clear();
}

// ZoneGrid 주기적 업데이트
void WorldManager::SectorUpdateWorkerThread()
{
	// 섹터에서 오브젝트 삭제
	DeleteObjSector();

	// 섹터 업데이트
	UpdateSector();

	// 빈 sector 정리
	ClearAllEmptySector();
	
	// 모든 플레이어 시야 업데이트
	UpdatePlayerVisible();
}

// Sector별 오브젝트 추가
void WorldManager::AddObjectToSector(const GameObjectRef obj, const ZoneType zoneType, const GridIndex& gridIndex)
{
	Sector* sector = FindSector(zoneType);
	if (sector == nullptr)
	{
		return;
	}
	// 그리드 범위 유효성 검사
	if (IsValidGridIndex(gridIndex) == false)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*sector);

		// Sector에 오브젝트 추가
		SectorId sectorId(gridIndex);
		auto& objectSector = sector->sectors[sectorId];
		objectSector.insert({ obj->GetObjectId(), obj });
	}
}

// 섹터에서 오브젝트 삭제 요청
void WorldManager::RequestSectorDelete(const ObjectId objectId, const ZoneType zoneType, const GridIndex& gridIndex)
{
	LOCK_GUARD;

	_forceDeleteObjs.push_back({ objectId , zoneType, gridIndex });
}


// 오브젝트 제거
void WorldManager::RemoveObjectToSector(const ObjectId objId, const ZoneType zoneType, const GridIndex& gridIndex)
{
	Sector* sector = FindSector(zoneType);
	if (sector == nullptr)
	{
		return;
	}
	// 그리드 범위 유효성 검사
	if (IsValidGridIndex(gridIndex) == false)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*sector);

		// 셀에 오브젝트 제거
		SectorId sectorId(gridIndex);

		auto sectorIt = sector->sectors.find(sectorId);
		if (sectorIt != sector->sectors.end())
		{
			auto& objectSector = sectorIt->second;
			objectSector.erase(objId);

			// 빈 sector 제거
			if (objectSector.empty())
			{
				sector->sectors.erase(sectorIt);
			}
		}
	}
}

// 업데이트할 섹터 insert
void WorldManager::RequestSectorUpdate(const GameObjectRef obj)
{
	LOCK_GUARD;

	_sectorUpdateSet.insert(obj);
}

// 월드 좌표로 그리드 좌표 변환
GridIndex WorldManager::MakeGridIndex(const float worldX, const float worldY) const
{
	const float gridX = (worldX - _mapData.worldMinX) / _mapData.gridSize;
	const float gridY = (worldY - _mapData.worldMinY) / _mapData.gridSize;
	return GridIndex(static_cast<int32>(gridX), static_cast<int32>(gridY));
}

GridIndex WorldManager::MakeGridIndex(const Vector3d& position) const
{
	const float gridX = (position.x - _mapData.worldMinX) / _mapData.gridSize;
	const float gridY = (position.y - _mapData.worldMinY) / _mapData.gridSize;
	return GridIndex(static_cast<int32>(gridX), static_cast<int32>(gridY));
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
			_zones[zoneType]->SetSpawnOffset(_mapData.spawnOffset);
			break;

		case ZoneType::Pvp:
			_zones[zoneType] = ObjectPool<PvpZone>::MakeUnique(zoneInfo);
			break;
		
		case ZoneType::Pve:
			_zones[zoneType] = ObjectPool<PveZone>::MakeUnique(zoneInfo);
			break;
		
		case ZoneType::Boss:
			_zones[zoneType] = ObjectPool<BossZone>::MakeUnique(zoneInfo);
			break;
		}
	}
}

// Zone별 Grid 초기화
void WorldManager::InitSectors()
{
	for (const ZoneInfo& zoneInfo : _mapData.zones)
	{
		_zoneSectors[zoneInfo.zoneType] = ObjectPool<Sector>::MakeUnique(zoneInfo);
	}
}

// 그리드좌표 유효성 검사
bool WorldManager::IsValidGridIndex(const GridIndex& gridIndex) const
{
	if (gridIndex.x < 0 || gridIndex.x >= _mapData.gridX || gridIndex.y < 0 || gridIndex.y >= _mapData.gridY)
	{
		return false;
	}
	return true;
}

// 시야 내의 GameObject 목록 가져오기
void WorldManager::GetVisibleObjectsInSector(ZoneType zoneType, const Vector3d& position, Vector<GameObjectRef>& outObjects, bool onlyPlayer /*= true*/)
{
	Sector* sector = FindSector(zoneType);
	if (sector == nullptr)
	{
		return;
	}
	
	GridIndex gridIndex = MakeGridIndex(position);
	Vector<SectorId> sectorIds;
	GetVisibleSectorIds(gridIndex, sectorIds);
	
	Vector<GameObjectRef> gameObjects;
	gameObjects.reserve(sectorIds.size());
	
	{
		GROUP_LOCK_GUARD(*sector);

		// 시야범위 내 오브젝트 가져오기
		for (const SectorId& sectorId : sectorIds)
		{
			auto sectorIt = sector->sectors.find(sectorId);
			if (sectorIt == sector->sectors.end())
			{
				continue;
			}

			const auto& objSectors = sectorIt->second;
			for (const auto& objSector : objSectors)
			{
				gameObjects.push_back(objSector.second);
			}
		}
	}

	for (const GameObjectRef& obj : gameObjects)
	{
		GridIndex objGrid = obj->GetCurrentGrid();

		// 그리드 범위 체크
		if (IsInGridRange(gridIndex, objGrid) == false)
		{
			continue;
		}

		if (obj->IsPlayer())
		{
			const PlayerRef& player = std::static_pointer_cast<Player>(obj);
			if (player->IsValid())
			{
				outObjects.push_back(player);
			}
		}
		else
		{
			if (onlyPlayer == false)
			{
				outObjects.push_back(obj);
			}
		}
	}
}

// Zone 내의 GameObject 목록 가져오기
void WorldManager::GetVisibleObjectsInZone(ZoneType zoneType, Vector<GameObjectRef>& outObjects)
{
	Sector* findSector = FindSector(zoneType);
	if (findSector == nullptr)
	{
		return;
	}
	{
		GROUP_LOCK_GUARD(*findSector);

		// sector에 있는 전체 GameObject 목록 가져오기
		auto& sectors = findSector->sectors;
		for (auto& sector : sectors)
		{
			ObjectSector& objSector = sector.second;

			for (auto it = objSector.begin(); it != objSector.end(); ++it)
			{
				GameObjectRef& obj = it->second;
				if (obj->IsPlayer())
				{
					const PlayerRef& player = std::static_pointer_cast<Player>(obj);
					if (player->IsValid())
					{
						outObjects.push_back(obj);
					}
				}
			}
		}
	}
}

// 시야 내의 GameObject 목록 가져오기
// 여러 Zone, Sector 검색
void WorldManager::GetVisibleObjectsInSectors(const Vector3d& position, Vector<GameObjectRef>& outObjects, bool onlyPlayer /*= true*/)
{
	Vector<ZoneType> zoneTypes;
	GetVisibleZones(position, zoneTypes);

	// 각 zoneType별 시야내에 속하는 object 가져오기
	for (ZoneType zoneType : zoneTypes)
	{
		GetVisibleObjectsInSector(zoneType, position, outObjects, onlyPlayer);
	}
}

// 월드좌표가 유효한지 판별
bool WorldManager::IsValidWorldPosition(const Protocol::PosInfo& posInfo) const
{
	if ((_mapData.worldMinX > posInfo.x() && _mapData.worldMaxX < posInfo.x()) ||
		_mapData.worldMinY > posInfo.y() && _mapData.worldMaxY < posInfo.y())
	{
		return false;
	}
	return true;
}

// 이동가능한 지점인지 판별
// - 월드좌표로 인자를 받고, 내부에서 그리드 좌표로 변환해서 판별
bool WorldManager::CanGo(float worldX, float worldY)
{
	Vector3d worldVector(worldX, worldY);
	
	// 유효성 검사
	GridIndex gridIndex = MakeGridIndex(worldVector);
	if (IsValidGridIndex(gridIndex) == false)
	{
		return false;
	}
	// 이동가능여부
	return _mapData.mapGrid[gridIndex.y][gridIndex.x];
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

// 시야내의 SectorId 가져오기
void WorldManager::GetVisibleSectorIds(const GridIndex& gridIndex, Vector<SectorId>& outSectorIds)
{
	int32 minGridX = max(0, gridIndex.x - playerVisibleRange);
	int32 maxGridX = min(_mapData.gridX, gridIndex.x + playerVisibleRange);
	int32 minGridY = max(0, gridIndex.y - playerVisibleRange);
	int32 maxGridY = min(_mapData.gridY, gridIndex.y + playerVisibleRange);

	// 그리드 좌표를 섹터 좌표로 변환
	int32 startSectorX = minGridX / SECTOR_SIZE;
	int32 endSectorX = maxGridX / SECTOR_SIZE;
	int32 startSectorY = minGridY / SECTOR_SIZE;
	int32 endSectorY = maxGridY / SECTOR_SIZE;

	for (int32 sectorY = startSectorY; sectorY <= endSectorY; sectorY++)
	{
		for (int32 sectorX = startSectorX; sectorX <= endSectorX; sectorX++)
		{
			outSectorIds.emplace_back(sectorX, sectorY);
		}
	}
}

// 시야 범위에 Zone이 속하는지 여부
bool WorldManager::IsInRange(const Vector3d& position, const ZonePos& worldPos, int32 gridSize)
{
	int32 playerWorldRange = playerVisibleRange * gridSize;
	float playerMinX = position.x - playerWorldRange;
	float playerMaxX = position.x + playerWorldRange;
	float playerMinY = position.y - playerWorldRange;
	float playerMaxY = position.y + playerWorldRange;

	// 모든 축에서 겹치는지 확인
	bool xOverlap = (playerMinX < worldPos.maxX) && (playerMaxX > worldPos.minX);
	bool yOverlap = (playerMinY < worldPos.maxY) && (playerMaxY > worldPos.minY);

	return xOverlap && yOverlap;
}

// 그리드 범위 체크
bool WorldManager::IsInGridRange(const GridIndex& gridIndex, const GridIndex& target)
{
	int32 dx = abs(gridIndex.x - target.x);
	int32 dy = abs(gridIndex.y - target.y);
	return (dx <= playerVisibleRange && dy <= playerVisibleRange);
}

// 섹터에서 오브젝트 삭제
void WorldManager::DeleteObjSector()
{
	Vector<ForceDeleteObject> _deleteObjs;
	{
		LOCK_GUARD;

		_deleteObjs = std::move(_forceDeleteObjs);
	}

	for (ForceDeleteObject& obj : _deleteObjs)
	{
		RemoveObjectToSector(obj.objectId, obj.zoneType, obj.gridIndex);
	}
}

// 큐에 쌓인 섹터 업데이트
void WorldManager::UpdateSector()
{
	Vector<GameObjectRef> gameObjects;
	gameObjects.reserve(_sectorUpdateSet.size());

	{
		LOCK_GUARD;
		
		for (auto it = _sectorUpdateSet.begin(); it != _sectorUpdateSet.end();)
		{
			GameObjectRef obj = *it;
			if (obj == nullptr)
			{
				it = _sectorUpdateSet.erase(it);
				continue;
			}

			gameObjects.push_back(obj);
			it = _sectorUpdateSet.erase(it);
		}
	}

	Vector<GameObjectRef> updateObjs;
	updateObjs.reserve(gameObjects.size());

	// 업데이트할 오브젝트 거르기
	for (GameObjectRef& obj : gameObjects)
	{
		ObjectPosition allPosition = obj->GetAllObjectPosition();
		
		// 이전 섹터와 현재 섹터가 같다면 섹터 이동 x
		if (allPosition.prevGrid == allPosition.currentGrid &&
			allPosition.prevZone == allPosition.currentZone)
		{
			continue;
		}

		updateObjs.push_back(obj);
	}

	// 섹터 업데이트
	for (GameObjectRef& obj : updateObjs)
	{
		ObjectPosition allPosition = obj->GetAllObjectPosition();

		// 이전 섹터에서 제거
		RemoveObjectToSector(obj->GetObjectId(), allPosition.prevZone, allPosition.prevGrid);
		// 새로운 섹터에 추가
		AddObjectToSector(obj, allPosition.currentZone, allPosition.currentGrid);
	}
}

// 월드 좌표로 Zone 타입 찾기
ZoneType WorldManager::GetZoneByPosition(const Vector3d& position) const
{
	for (const ZoneInfo& zoneInfo : _mapData.zones)
	{
		const ZonePos& zonePos = zoneInfo.worldPos;

		if (position.x >= zonePos.minX && position.x < zonePos.maxX
			&& position.y >= zonePos.minY && position.y < zonePos.maxY)
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

// Sector 가져오기
Sector* WorldManager::FindSector(const ZoneType zoneType)
{
	auto findit = _zoneSectors.find(zoneType);
	if (findit != _zoneSectors.end())
	{
		return findit->second.get();
	}
	CRASH("NOT INIT SECTOR!!");
	return nullptr;
}

// 빈 sector 정리
void WorldManager::ClearAllEmptySector()
{
	for (auto& sector : _zoneSectors)
	{
		Sector* getSector = sector.second.get();
		GROUP_LOCK_GUARD(*getSector);

		for (auto it = getSector->sectors.begin(); it != getSector->sectors.end();)
		{
			if (it->second.empty())
			{
				it = getSector->sectors.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

// 모든 플레이어 시야 업데이트
void WorldManager::UpdatePlayerVisible()
{
	// 모든 활성 세션 가져오기
	Vector<SessionRef> sessions;
	SessionMgr.GetActiveSessions(sessions);

	for (SessionRef& session : sessions)
	{
		Player* player = session->GetPlayer<Player>();
		if (player == nullptr || player->IsValid() == false)
		{
			continue;
		}

		// 캐릭터의 시야내 오브젝트 가져오기
		Vector<GameObjectRef> currentVisible;
		GetVisibleObjectsInSectors(player->GetWorldPosition(), currentVisible, false);

		// 시야 업데이트
		player->UpdateVision(currentVisible);
	}
}
