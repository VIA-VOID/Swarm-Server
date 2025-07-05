#pragma once
#include "Object/GameObject.h"
#include "Zone/WorldManager.h"

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
	// 게임 입장, 스폰
	void EnterGame(const ZoneType zoneType);
	// 세션 가져오기
	SessionRef GetSession();
	// 유효성 검사
	bool IsValid() const;
	
private:
	// 초기 스텟 정보 초기화
	void InitStatInfo();
	// 게임 입장 패킷 전송
	void SendEnterGamePkt(const ZoneType zoneType, Protocol::ObjectInfo& outObjInfo);
	// playerType(직업) 정보 가져오기
	Protocol::PlayerType GetPlayerType() const;
	// 본인에게 sendPacket
	template <typename T>
	void SendUnicast(const T& message, PacketID pktId);
	// 단일대상 sendPacket
	template <typename T>
	void SendUnicast(SessionRef targetSession, const T& message, PacketID pktId);
	// 복수대상 sendPacket
	template <typename T>
	void SendBroadcast(const Vector<PlayerRef>& players, const T& message, PacketID pktId, bool isSelf = false);
	// 시야 내 플레이어들에게 패킷 전송
	template <typename T>
	void SendBroadcastToVisiblePlayers(const T& message, PacketID pktId, bool isSelf = false);

private:
	// 캐릭터 정보
	SessionRef _session;
	Protocol::PlayerType _playerType;

	// flag값
	std::atomic<bool> _isValid;
};

// 본인에게 sendPacket
template <typename T>
void Player::SendUnicast(const T& message, PacketID pktId)
{
	if (IsValid() == false)
	{
		return;
	}
	PacketHandler::SendPacket(_session, message, pktId);
}

// 단일대상 sendPacket
template <typename T>
void Player::SendUnicast(SessionRef targetSession, const T& message, PacketID pktId)
{
	if (targetSession == nullptr || targetSession->IsClosed())
	{
		return;
	}
	PacketHandler::SendPacket(targetSession, message, pktId);
}

// 복수대상 sendPacket
template <typename T>
void Player::SendBroadcast(const Vector<PlayerRef>& players, const T& message, PacketID pktId, bool isSelf/* = false*/)
{
	for (const auto& player : players)
	{
		if (isSelf == false && player->GetObjectId() == GetObjectId())
		{
			continue;
		}
		SendUnicast(player->GetSession(), message, pktId);
	}
}

// 시야 내 플레이어들에게 패킷 전송
template <typename T>
void Player::SendBroadcastToVisiblePlayers(const T& message, PacketID pktId, bool isSelf/* = false*/)
{
	Vector<PlayerRef> players;
	WorldMgr.GetVisiblePlayersInZones(GetWorldPosition(), players);

	SendBroadcast(players, message, pktId, isSelf);
}
