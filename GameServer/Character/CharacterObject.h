#pragma once

class CharacterId;

/*-------------------------------------------------------
				CharacterObject

- 모든 캐릭터의 공통 항목
- ACharacterObject 상속받아 플레이어, 몬스터 구현
--------------------------------------------------------*/

class CharacterObject : public std::enable_shared_from_this<CharacterObject>
{
public:
	CharacterObject();
	virtual ~CharacterObject();
	// CharacterId 가져오기
	CharacterId GetCharacterId();

protected:
	// 캐릭터 공용 정보(Protocol::CharacterInfo) 만들기
	Protocol::CharacterInfo MakeCharacterInfo();
	

protected:
	// 고유 ID
	CharacterId _characterId;
	// 캐릭터 타입
	Protocol::CharacterType _type;
	// 위치 및 방향 정보
	Protocol::PosInfo _posInfo;
	// 캐릭터 상태
	Protocol::CharacterState _state;
	// 스탯 정보
	Protocol::StatInfo _statInfo;
	// 이름
	std::string _name;

};
