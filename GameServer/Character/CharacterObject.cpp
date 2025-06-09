#include "pch.h"
#include "CharacterObject.h"

/*----------------------------
		CharacterObject
----------------------------*/

CharacterObject::CharacterObject()
	: _characterId(CharacterId::Generate()), _type(Protocol::CharacterType::Character_TYPE_NONE), 
	_state(Protocol::CharacterState::Character_STATE_NONE)
{
}

CharacterObject::~CharacterObject()
{
}

// 캐릭터 공용 정보 만들기
Protocol::CharacterInfo CharacterObject::MakeCharacterInfo()
{
	Protocol::CharacterInfo characterInfo;
	characterInfo.set_characterid(_characterId.GetId());
	characterInfo.set_type(_type);
	*characterInfo.mutable_posinfo() = _posInfo;
	characterInfo.set_state(_state);
	*characterInfo.mutable_statinfo() = _statInfo;
	characterInfo.set_name(_name);

	return characterInfo;
}

// CharacterId 가져오기
CharacterId CharacterObject::GetCharacterId()
{
	return _characterId;
}
