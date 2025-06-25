#pragma once
#include "Object/GameObject.h"

/*-------------------------------------------------------
				Player
--------------------------------------------------------*/

class Player : public GameObject
{
public:
	Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name);
	virtual ~Player();

	// 초기 스텟 정보 초기화
	void InitStatInfo();
	// 세션 참조 해제
	void DetachSession();
	// 세션 가져오기
	SessionRef GetSession();
	// 플레이어 정보(Protocol::PlayerInfo) 만들기
	void MakePlayerInfo(Protocol::PlayerInfo& outPlayerInfo);

private:
	// 캐릭터 정보
	SessionRef _session;
	Protocol::PlayerType _playerType;
};
