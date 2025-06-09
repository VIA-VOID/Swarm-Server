#include "pch.h"
#include "Player.h"
#include "Map/MapDefine.h"

/*----------------------------
		Player
----------------------------*/

Player::Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name)
	: _session(session), _playerType(playerType), _currentMap(MapIds::Invalid)
{
	_type = Protocol::Character_TYPE_PLAYER;
	_name = name;
}

Player::~Player()
{
}

// 세션 참조 해제
void Player::DetachSession()
{
	_session.reset();
	_session = nullptr;
}

// 맵 이동
void Player::MoveMap(MapId targetMapId)
{
	_currentMap = targetMapId;
}

// 세션 가져오기
SessionRef Player::GetSession()
{
	return _session;
}

// 현재 있는 맵
MapId Player::CurrentMap() const
{
	return _currentMap;
}

// 포지션 설정
void Player::SetPosition(const Protocol::PosInfo& posInfo)
{
	_posInfo = posInfo;
}

// 플레이어 정보(Protocol::PlayerInfo) 만들기
Protocol::PlayerInfo Player::MakePlayerInfo()
{
	Protocol::PlayerInfo playerInfo;
	*playerInfo.mutable_info() = MakeCharacterInfo();
	playerInfo.set_playertype(_playerType);

	return playerInfo;
}
