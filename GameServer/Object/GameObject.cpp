#include "pch.h"
#include "GameObject.h"
#include "ObjectDefine.h"

GameObject::GameObject()
	: _objectId(ObjectId::Generate()), _objectType(Protocol::ObjectType::OBJECT_TYPE_NONE),
	_objectState(Protocol::ObjectState::OBJECT_STATE_NONE)
{
}

GameObject::~GameObject()
{
}

// ObjectId 가져오기
ObjectId GameObject::GetObjectId()
{
	return _objectId;
}

// Object 공용 정보(Protocol::ObjectInfo) 만들기
Protocol::ObjectInfo GameObject::MakeObjectInfo()
{
	Protocol::ObjectInfo objectInfo;
	objectInfo.set_objectid(_objectId.GetId());
	objectInfo.set_type(_objectType);
	*objectInfo.mutable_posinfo() = _posInfo;
	objectInfo.set_state(_objectState);
	*objectInfo.mutable_statinfo() = _statInfo;
	objectInfo.set_name(_name);

	return objectInfo;
}
