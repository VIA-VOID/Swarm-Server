#include "pch.h"
#include "GameObject.h"
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
Vector3d GameObject::GetWorldPosition()
{
	LOCK_GUARD;

	return _vectorPos;
}

// 위치 가져오기(Protocol::PosInfo)
void GameObject::GetWorldPosition(Protocol::PosInfo& outPosInfo)
{
	LOCK_GUARD;

	outPosInfo.CopyFrom(_pos);
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Protocol::PosInfo& posInfo)
{
	Vector3d vectorPos(0, 0);
	vectorPos.UpdatePosition(posInfo);
	SetWorldPosition(vectorPos, posInfo);
}

// 위치 업데이트
void GameObject::SetWorldPosition(const Vector3d& vectorPos)
{
	Protocol::PosInfo posInfo = vectorPos.MakePosInfo();
	SetWorldPosition(vectorPos, posInfo);
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

void GameObject::SetWorldPosition(const Vector3d& vectorPos, const Protocol::PosInfo& posInfo)
{
	ZoneType newZoneType = WorldMgr.GetZoneByPosition(vectorPos);
	GridIndex newGridIndex = WorldMgr.MakeGridIndex(vectorPos);

	bool needSectorUpdate = false;
	{
		LOCK_GUARD;

		// 이전 정보 저장
		_prevZoneType = _zoneType;
		_prevGridIndex = _gridIndex;

		// 섹터 변경 여부
		needSectorUpdate = (_zoneType != newZoneType || _gridIndex != newGridIndex);

		// 위치 업데이트
		_vectorPos = vectorPos;
		_pos = posInfo;
		_zoneType = newZoneType;
		_gridIndex = newGridIndex;
	}
	// 섹터 변경시 섹터 업데이트 요청
	if (needSectorUpdate)
	{
		WorldMgr.RequestSectorUpdate(shared_from_this());
	}
}

// Object 공용 정보(Protocol::ObjectInfo) 만들기
void GameObject::MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo)
{
	LOCK_GUARD;

	outObjectInfo.set_objectid(_objectId.GetId());
	outObjectInfo.set_type(_objectType);
	outObjectInfo.mutable_posinfo()->CopyFrom(_pos);
	outObjectInfo.mutable_statinfo()->CopyFrom(_statInfo);
	outObjectInfo.set_name(_name);
}

// 전체 위치정보 가져오기
ObjectPosition GameObject::GetAllObjectPosition()
{
	LOCK_GUARD;

	return { _prevZoneType, _zoneType, _prevGridIndex, _gridIndex, _vectorPos };
}

GridIndex GameObject::GetCurrentGrid()
{
	LOCK_GUARD;

	return _gridIndex;
}
