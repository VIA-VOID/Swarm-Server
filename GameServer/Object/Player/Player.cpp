#include "pch.h"
#include "Player.h"
#include "Object/Stat/StatManager.h"

/*----------------------------
		Player
----------------------------*/

Player::Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name)
	: _session(session), _playerType(playerType)
{
	_objectType = Protocol::OBJECT_TYPE_PLAYER;
	_name = name;
}

Player::~Player()
{
}

// 초기 스텟 정보 초기화
void Player::InitStatInfo()
{
	StatMgr.GetPlayerStat(_playerType, _statInfo);
}

// 세션 참조 해제
void Player::DetachSession()
{
	_session.reset();
	_session = nullptr;
}

// 세션 가져오기
SessionRef Player::GetSession()
{
	return _session;
}

// 플레이어 정보(Protocol::PlayerInfo) 만들기
void Player::MakePlayerInfo(Protocol::PlayerInfo& outPlayerInfo)
{
	Protocol::ObjectInfo objectInfo;
	MakeObjectInfo(objectInfo);

	outPlayerInfo.mutable_info()->CopyFrom(objectInfo);
	outPlayerInfo.set_playertype(_playerType);
}
