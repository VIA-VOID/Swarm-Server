#include "pch.h"
#include "GameObject.h"
#include "Zone/Vector3d.h"
#include "Zone/WorldManager.h"

// ObjectId 가져오기
GameObject::GameObject()
	: _objectId(ObjectId::Generate()), _objectType(Protocol::ObjectType::OBJECT_TYPE_NONE),
	 _vectorPos(0, 0, 0), _gridIndex(0, 0), _zoneType(ZoneType::Town)
{
}

GameObject::~GameObject()
{
}

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
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Vector3d& vectorPos)
{
	_vectorPos = vectorPos;
	_pos.set_x(vectorPos.GetWorldX());
	_pos.set_y(vectorPos.GetWorldY());
	_pos.set_z(vectorPos.GetWorldZ());
	_pos.set_yaw(vectorPos.GetWorldYaw());
}

// zoneType & GridIndex 업데이트
void GameObject::SetZoneGridIndex(const ZoneType zoneType, const GridIndex& gridIndex)
{
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

GridIndex GameObject::GetCurrentGrid() const
{
	return GridIndex();
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
