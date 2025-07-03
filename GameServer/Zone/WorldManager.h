#pragma once
#include "ZoneDefine.h"
#include "BaseZone.h"
#include "ZoneGrid.h"

/*--------------------------------------------------------
					WorldManager

- 월드 맵 관리
- Zone으로 나누어진 맵 데이터(좌표) 관리
- 좌표는 Vector3d 클래스 사용
--------------------------------------------------------*/
class WorldManager : public Singleton<WorldManager>
{
public:
	void Init() override;
	void Shutdown() override;

	// ZoneGrid 주기적 업데이트
	void ZoneGridUpdateWorkerThread();
	// Zone별 오브젝트 추가
	void AddObjectToZone(GameObjectRef obj, const ZoneType zoneType, const GridIndex& gridIndex);
	// 오브젝트 제거
	void RemoveObjectToZone(const ObjectId objId, const ZoneType zoneType, const GridIndex& gridIndex);
	// 시야 내의 플레이어 목록 가져오기
	// 여러 Zone 검색
	void GetVisiblePlayersInZones(const Vector3d& position, Vector<PlayerRef>& outPlayers);

	// zone 가져오기
	BaseZone* FindZone(const ZoneType zoneType);
	// 월드 좌표로 Zone 타입 찾기
	ZoneType GetZoneByPosition(const Vector3d& position) const;
	// zoneGrid 가져오기
	ZoneGrid* FindZoneGrid(const ZoneType zoneType);

private:
	// Zone 초기화
	void InitZones();
	// Zone별 Grid 초기화
	void InitZoneGrids();
	// 그리드좌표 유효성 검사
	bool IsVaildGridIndex(const GridIndex& gridIndex) const;
	// 시야 내의 플레이어 목록 가져오기
	// 단일 Zone 검색
	void GetVisiblePlayersInZone(ZoneType zoneType, const Vector3d& position, Vector<PlayerRef>& outPlayers);
	// 시야 범위에 포함되는 Zone 목록 가져오기
	void GetVisibleZones(const Vector3d& position, Vector<ZoneType>& outZoneTypes);
	// 시야 범위에 Zone이 속하는지 여부
	bool IsInRange(const Vector3d& position, const ZonePos& worldPos, int32 gridSize);

private:
	USE_LOCK;
	// 맵 데이터
	MapData _mapData;
	// Zone별 공간 분할 그리드
	HashMap<ZoneType, ZoneGridURef> _zoneGrids;
	// Zone 관리
	HashMap<ZoneType, BaseZoneURef> _zones;
};
