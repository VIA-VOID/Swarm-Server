#pragma once
#include "ObjectDefine.h"

/*-------------------------------------------------------
				GameObject

- Object 최상위 클래스
- Player, Monster 등 상속받아서 구현
--------------------------------------------------------*/
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	// 유효한지 여부 확인
	virtual bool IsValid() const;
	// ObjectId 가져오기
	ObjectId GetObjectId() const;
	// 위치 가져오기
	Vector3d GetWorldPosition();
	void GetWorldPosition(Protocol::PosInfo& outPosInfo);
	// 위치 업데이트
	void SetWorldPosition(const Protocol::PosInfo& posInfo);
	void SetWorldPosition(const Vector3d& vectorPos);	
	// Player인지 확인
	bool IsPlayer() const;
	// Monster인지 확인
	bool IsMonster() const;
	// Object 공용 정보(Protocol::ObjectInfo) 만들기
	void MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo);
	
	// 전체 위치정보 가져오기
	ObjectPosition GetAllObjectPosition();
	GridIndex GetCurrentGrid();

protected:
	void SetWorldPosition(const Vector3d& vectorPos, const Protocol::PosInfo& posInfo);

protected:
	USE_LOCK;

	// 고유 ID
	ObjectId _objectId;
	// Object 타입
	Protocol::ObjectType _objectType;
	// 위치 및 방향 정보
	Protocol::PosInfo _pos;
	Vector3d _vectorPos;
	GridIndex _gridIndex;
	// Zone 위치정보
	ZoneType _zoneType;
	// 스탯 정보
	Protocol::StatInfo _statInfo;
	// 이름
	std::string _name;
	// 유효한지 여부
	std::atomic<bool> _isValid;
	// 이전 정보
	GridIndex _prevGridIndex;
	ZoneType _prevZoneType;
};
