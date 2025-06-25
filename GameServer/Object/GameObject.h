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

	// ObjectId 가져오기
	ObjectId GetObjectId() const;
	// 위치 가져오기
	void GetWorldPosition(Vector3d& outVector3d) const;
	void GetWorldPosition(Protocol::PosInfo& outPosInfo) const;
	// 위치 업데이트
	void SetWorldPosition(const Protocol::PosInfo& posInfo);
	void SetWorldPosition(const Vector3d& vectorPos);
	// Player인지 확인
	bool IsPlayer() const;
	// Monster인지 확인
	bool IsMonster() const;

protected:
	// Object 공용 정보(Protocol::ObjectInfo) 만들기
	void MakeObjectInfo(Protocol::ObjectInfo& outObjectInfo);

protected:
	// 고유 ID
	ObjectId _objectId;
	// Object 타입
	Protocol::ObjectType _objectType;
	// 위치 및 방향 정보
	Protocol::PosInfo _pos;
	Vector3d _vectorPos;
	// Object 상태
	Protocol::ObjectState _objectState;
	// 스탯 정보
	Protocol::StatInfo _statInfo;
	// 이름
	std::string _name;
};
