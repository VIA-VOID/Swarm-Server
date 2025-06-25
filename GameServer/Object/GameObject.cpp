#include "pch.h"
#include "GameObject.h"

GameObject::GameObject()
	: _objectId(ObjectId::Generate()), _objectType(Protocol::ObjectType::OBJECT_TYPE_NONE),
	_objectState(Protocol::ObjectState::OBJECT_STATE_NONE), _vectorPos()
{
}

GameObject::~GameObject()
{
}

// ObjectId 가져오기
ObjectId GameObject::GetObjectId() const
{
	return _objectId;
}

// 위치 가져오기(Vector3d)
void GameObject::GetWorldPosition(Vector3d& outVector3d) const
{
	outVector3d = _vectorPos;
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
	_vectorPos.SetWorldX(posInfo.x());
	_vectorPos.SetWorldY(posInfo.y());
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Vector3d& vectorPos)
{
	_vectorPos.SetWorldX(vectorPos.GetWorldX());
	_vectorPos.SetWorldY(vectorPos.GetWorldY());
	_pos.set_x(vectorPos.GetWorldX());
	_pos.set_x(vectorPos.GetWorldY());
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

// Object 공용 정보(Protocol::ObjectInfo) 만들기
void GameObject::MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo)
{
	outObjectInfo.set_objectid(_objectId.GetId());
	outObjectInfo.set_type(_objectType);
	outObjectInfo.mutable_posinfo()->CopyFrom(_pos);
	outObjectInfo.set_state(_objectState);
	outObjectInfo.mutable_statinfo()->CopyFrom(_statInfo);
	outObjectInfo.set_name(_name);
}
