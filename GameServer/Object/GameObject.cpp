#include "pch.h"
#include "GameObject.h"
#include "World/Zone/Vector3d.h"
#include "World/WorldManager.h"

// ObjectId 가져오기
GameObject::GameObject()
	: _objectId(ObjectId::Generate()), _objectType(Protocol::ObjectType::OBJECT_TYPE_NONE),
	 _vectorPos(0, 0), _gridIndex(0, 0), _zoneType(ZoneType::Town), _prevGridIndex(0, 0), _prevZoneType(ZoneType::Town)
{
	_isValid.store(true, std::memory_order::memory_order_relaxed);
}

GameObject::~GameObject()
{
}

// 유효한지 여부 확인
bool GameObject::IsValid() const
{
	return _isValid.load();
}

// ObjectId 가져오기
ObjectId GameObject::GetObjectId() const
{
	return _objectId;
}

// 위치 가져오기(Vector3d)
Vector3d GameObject::GetWorldPosition() const
{
	return _vectorPos;
}

// 위치 가져오기(Protocol::PosInfo)
void GameObject::GetWorldPosition(Protocol::PosInfo& outPosInfo) const
{
	outPosInfo.CopyFrom(_pos);
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Protocol::PosInfo& posInfo)
{
	_pos.CopyFrom(posInfo);
	_vectorPos.UpdatePosition(posInfo);
	ZoneType zoneType = WorldMgr.GetZoneByPosition(_vectorPos);
	GridIndex gridIndex = WorldMgr.MakeGridIndex(_vectorPos);
	SetZoneGridIndex(zoneType, gridIndex);
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Vector3d& vectorPos)
{
	_vectorPos = vectorPos;
	_pos = vectorPos.MakePosInfo();
	ZoneType zoneType = WorldMgr.GetZoneByPosition(_vectorPos);
	GridIndex gridIndex = WorldMgr.MakeGridIndex(_vectorPos);
	SetZoneGridIndex(zoneType, gridIndex);
}

// zoneType & GridIndex 업데이트
void GameObject::SetZoneGridIndex(const ZoneType zoneType, const GridIndex& gridIndex)
{
	_prevZoneType = _zoneType;
	_prevGridIndex = _gridIndex;

	// 섹터 변경시 섹터 업데이트 요청
	if (_zoneType != zoneType || _gridIndex != gridIndex)
	{
		WorldMgr.RequestSectorUpdate(shared_from_this());
	}

	_zoneType = zoneType;
	_gridIndex = gridIndex;
}

// 모든 위치정보 업데이트
void GameObject::SetAllPosition()
{
	// todo
}

// Player인지 확인
bool GameObject::IsPlayer() const
{
	return _objectType == Protocol::ObjectType::OBJECT_TYPE_PLAYER;
}

// Monster인지 확인
bool GameObject::IsMonster() const
{
	return _objectType == Protocol::ObjectType::OBJECT_TYPE_MONSTER;
}

ZoneType GameObject::GetCurrentZone() const
{
	return _zoneType;
}

ZoneType GameObject::GetPrevZone() const
{
	return _prevZoneType;
}

GridIndex GameObject::GetCurrentGrid() const
{
	return _gridIndex;
}

GridIndex GameObject::GetPrevGrid() const
{
	return _prevGridIndex;
}

// Object 공용 정보(Protocol::ObjectInfo) 만들기
void GameObject::MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo, Protocol::PlayerType playerType)
{
	MakeObjectInfo(outObjectInfo);
	outObjectInfo.set_playertype(playerType);
	outObjectInfo.set_monstertype(Protocol::MonsterType::MONSTER_TYPE_NONE);
}

void GameObject::MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo, Protocol::MonsterType monsterType)
{
	MakeObjectInfo(outObjectInfo);
	outObjectInfo.set_playertype(Protocol::PlayerType::PLAYER_TYPE_NONE);
	outObjectInfo.set_monstertype(monsterType);
}

void GameObject::MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo)
{
	outObjectInfo.set_objectid(_objectId.GetId());
	outObjectInfo.set_type(_objectType);
	outObjectInfo.mutable_posinfo()->CopyFrom(_pos);
	outObjectInfo.mutable_statinfo()->CopyFrom(_statInfo);
	outObjectInfo.set_name(_name);
}
