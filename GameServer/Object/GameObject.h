#pragma once

class ObjectId;

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
	ObjectId GetObjectId();

protected:
	// Object 공용 정보(Protocol::ObjectInfo) 만들기
	Protocol::ObjectInfo MakeObjectInfo();

protected:
	// 고유 ID
	ObjectId _objectId;
	// Object 타입
	Protocol::ObjectType _objectType;
	// 위치 및 방향 정보
	Protocol::PosInfo _posInfo;
	// Object 상태
	Protocol::ObjectState _objectState;
	// 스탯 정보
	Protocol::StatInfo _statInfo;
	// 이름
	std::string _name;

};
