#pragma once
#include "Object/GameObject.h"
#include "Utils/Timer.h"
#include "Chat/Chat.h"

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
	// 채팅 브로드캐스트
	void ChatMessage(const Protocol::CS_CHAT_MSG& packet);

	// 세션 가져오기
	SessionRef GetSession();
	// 유효성 검사
	bool IsValid() const override;
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
	// 시야 내 새로 들어온 Object 스폰
	void SendSpawnPacket(const Vector<GameObjectRef>& currentVisible, const HashSet<ObjectId>& newVisible);
	// 시야 내 제거된 Object 디스폰
	void SendDespawnPacket(const HashSet<ObjectId>& newVisible);
	// 채팅 브로드캐스트
	// - Job 스레드로 넘김
	template <typename Func>
	void BroadcastChatAsync(const std::string& msg, Protocol::MsgType msgType, Func&& func);

private:
	SessionRef _session;
	HashSet<ObjectId> _visibleObjects;

	Protocol::PlayerType _playerType;
	
	TimePoint _lastMoveTime;
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

// 채팅 브로드캐스트
// - Job 스레드로 넘김
template <typename BroadcastFunc>
void Player::BroadcastChatAsync(const std::string& msg, Protocol::MsgType msgType, BroadcastFunc&& func)
{
	JobQ.DoAsync([self = shared_from_this(), msg, msgType, playerName = _name, func = std::forward<BroadcastFunc>(func)]() {
		Protocol::SC_CHAT_MSG chatPkt;
		chatPkt.set_msg(Chat::MakeMessage(msg, playerName, msgType));
		chatPkt.set_msgtype(msgType);
		func(self, chatPkt);
	}, JobGroup::Social);
}
