#pragma once
#include "ZoneDefine.h"
#include "BaseZone.h"

/*--------------------------------------------------------
					WorldManager

- 월드 맵 관리
- Zone으로 나누어진 맵 데이터(좌표) 관리
- 좌표는 Vector2D 클래스 사용
--------------------------------------------------------*/
class WorldManager : public Singleton<WorldManager>
{
public:
	void Init() override;
	void Shutdown() override;
	
	// 게임 입장
	bool EnterGame(GameObjectRef obj);
	

private:
	// Zone 초기화
	void InitZones();
	// 그리드좌표 유효성 검사
	bool IsVaildGridIndex(const GridIndex& gridIndex) const;
	// 셀에 오브젝트 추가
	bool AddObjectToCell(const GameObjectRef obj, const GridIndex& gridIndex);
	// zone 가져오기
	BaseZone* GetZone(const ZoneType zoneType);

private:
	USE_LOCK;
	// 맵 데이터
	MapData _mapData;
	// 공간 분할 그리드
	Vector<Vector<GridCell>> _grid;
	// Zone 관리
	HashMap<ZoneType, BaseZoneURef> _zones;
};
