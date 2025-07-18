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

	// Player 이동
	void Move(const Protocol::CS_PLAYER_MOVE& packet);
	// 세션 참조 해제
	void DetachSession();
	// 게임 입장, 스폰
	void EnterGame(const ZoneType zoneType);
	// 시야내의 ObjectId 업데이트
	void UpdateVision(Vector<GameObjectRef>& currentVisible);
	// 세션 가져오기
	SessionRef GetSession();
	// 유효성 검사
	bool IsValid() const;
	// 패킷전송
	template <typename T>
	void SendUnicast(const T& message, const PacketID pktId);
	
private:
	// 이동 시뮬레이션
	void MoveSimulate(const Protocol::CS_PLAYER_MOVE& packet);
	// 예측 이동
	void PredicateMove(const Protocol::CS_PLAYER_MOVE& packet, const float halfRttSec);
	// 초기 스텟 정보 초기화
	void InitStatInfo();
	// 게임 입장 패킷 전송
	void SendEnterGamePkt(const ZoneType zoneType, Protocol::ObjectInfo& outObjInfo);
	// playerType(직업) 정보 가져오기
	Protocol::PlayerType GetPlayerType() const;

private:
	SessionRef _session;
	HashSet<ObjectId> _visibleObjects;

	Protocol::PlayerType _playerType;
	
	TimePoint _lastMoveTime;
	std::atomic<bool> _isValid;
	std::atomic<bool> _isEntered;
};

// 패킷전송
template <typename T>
void Player::SendUnicast(const T& message, const PacketID pktId)
{
	if (_session == nullptr || _session->IsClosed())
	{
		return;
	}
	PacketHandler::SendPacket(_session, message, pktId);
}
