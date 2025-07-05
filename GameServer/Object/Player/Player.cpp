#include "pch.h"
#include "Player.h"
#include "Object/Stat/StatManager.h"
#include "Zone/TownZone.h"

/*----------------------------
		Player
----------------------------*/

Player::Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name)
	: _session(session), _playerType(playerType)
{
	_objectType = Protocol::OBJECT_TYPE_PLAYER;
	_name = name;
	_isValid.store(true, std::memory_order::memory_order_relaxed);
}

Player::~Player()
{
	if (_session != nullptr)
	{
		_session.reset();
	}
}

// 초기 스텟 정보 초기화
void Player::InitStatInfo()
{
	StatMgr.GetPlayerStat(_playerType, _statInfo);
}

// 세션 참조 해제
void Player::DetachSession()
{
	_isValid.store(false);
	_session.reset();
	_session = nullptr;
}

// 게임 입장, 스폰
void Player::EnterGame(const ZoneType zoneType)
{
	// 접속한 플레이어에게 게임 입장 패킷 전송
	Protocol::ObjectInfo objInfo;
	SendEnterGamePkt(zoneType, objInfo);
	
	// 시야 안에 있는 플레이어 목록
	Vector<PlayerRef> players;
	WorldMgr.GetVisiblePlayersInZones(GetWorldPosition(), players);

	// 시야 안에 있는 타 플레이어들에게 입장 알림
	{
		Protocol::SC_PLAYER_SPAWN spawnPkt;

		// 오브젝트 정보 만들기
		MakeObjectInfo(objInfo, _playerType);
		spawnPkt.mutable_objectinfo()->CopyFrom(objInfo);

		// 타 플레이어에게 현재 캐릭터 스폰 알림
		SendBroadcast(players, spawnPkt, PacketID::SC_PLAYER_SPAWN);
	}

	// 현재 접속한 플레이어에게 시야 안에 있는 타 플레이어 목록 알림
	for (const auto& player : players)
	{
		Protocol::SC_PLAYER_SPAWN spawnPkt;
		Protocol::ObjectInfo otherObjInfo;

		// 타 오브젝트 정보 만들기
		player->MakeObjectInfo(otherObjInfo, player->GetPlayerType());
		spawnPkt.mutable_objectinfo()->CopyFrom(otherObjInfo);

		// 현재 캐릭터에게 타 플레이어 존재 알림
		SendUnicast(spawnPkt, PacketID::SC_PLAYER_SPAWN);
	}
}

// 세션 가져오기
SessionRef Player::GetSession()
{
	if (IsValid())
	{
		return _session;
	}
	return nullptr;
}

// 유효성 검사
bool Player::IsValid() const
{
	return _isValid.load() && _session && _session->IsClosed() == false;
}

// 게임 입장 패킷 전송
void Player::SendEnterGamePkt(const ZoneType zoneType, Protocol::ObjectInfo& outObjInfo)
{
	TownZone* townZone = static_cast<TownZone*>(WorldMgr.FindZone(zoneType));

	// 플레이어 입장 패킷 생성
	Protocol::SC_PLAYER_ENTER_GAME enterGamePkt;

	// 위치 업데이트
	Vector3d spawnVector = townZone->GetRandomSpawnPosition();
	SetWorldPosition(spawnVector);

	// 초기 스텟 설정
	InitStatInfo();

	// 오브젝트 정보 만들기
	Protocol::ObjectInfo objInfo;
	MakeObjectInfo(objInfo, _playerType);

	outObjInfo = objInfo;

	// ObjectInfo 설정
	enterGamePkt.mutable_objectinfo()->CopyFrom(objInfo);

	// 입장 패킷 전송
	PacketHandler::SendPacket(_session, enterGamePkt, PacketID::SC_PLAYER_ENTER_GAME);

	// zone 업데이트
	ZonePos worldPos = townZone->GetWorldPosition();
	GridIndex gridIndex = spawnVector.MakeGridIndex(worldPos);
	WorldMgr.AddObjectToZone(shared_from_this(), zoneType, gridIndex);

	// 플레이어 zone, grid 업데이트
	SetZoneGridIndex(zoneType, gridIndex);
}

// playerType(직업) 정보 가져오기
Protocol::PlayerType Player::GetPlayerType() const
{
	return _playerType;
}
