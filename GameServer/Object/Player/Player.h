#pragma once
#include "Object/GameObject.h"
#include "Zone/MapDefine.h"

/*-------------------------------------------------------
				Player
--------------------------------------------------------*/

class Player : public GameObject
{
public:
	Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name);
	virtual ~Player();

	// 세션 참조 해제
	void DetachSession();
	// 맵 이동
	void MoveMap(MapId targetMapId);
	// 세션 가져오기
	SessionRef GetSession();
	// 현재 있는 맵 가져오기
	MapId CurrentMap() const;
	// 포지션 설정
	void SetPosition(const Protocol::PosInfo& posInfo);

	// 플레이어 정보(Protocol::PlayerInfo) 만들기
	Protocol::PlayerInfo MakePlayerInfo();

private:
	// 캐릭터 정보
	SessionRef _session;
	Protocol::PlayerType _playerType;

	// 캐릭터 상태
	MapId _currentMap;
};
