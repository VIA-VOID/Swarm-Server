#include "pch.h"
#include "Player.h"
#include "Object/Stat/StatManager.h"
#include "Zone/WorldManager.h"
#include "Zone/TownZone.h"
#include "Utils/Timer.h"

/*----------------------------
		Player
----------------------------*/

Player::Player(SessionRef session, const Protocol::PlayerType& playerType, const std::string& name)
	: _session(session), _playerType(playerType), _lastMoveTime(NOW)
{
	_objectType = Protocol::OBJECT_TYPE_PLAYER;
	_name = name;
	_isValid.store(true, std::memory_order::memory_order_relaxed);
	_isEntered.store(false, std::memory_order::memory_order_relaxed);
}

Player::~Player()
{
	if (_session != nullptr)
	{
		_session.reset();
	}
}

// Player 이동
void Player::Move(const Protocol::CS_PLAYER_MOVE& packet)
{
	Protocol::PosInfo posInfo = packet.posinfo();
	Protocol::PosMoveVector moveVector = packet.movevector();

	// 좌표 유효성 검사
	if (WorldMgr.IsValidWorldPosition(posInfo) == false)
	{
		LOG_ERROR("캐릭터 좌표 오류: ClientPos(" + std::to_string(posInfo.x()) + ", " + std::to_string(posInfo.y()) + "), " +
			"SessionID: " + std::to_string(_session->GetSessionID().GetID()));

		_session->Close();
	}
	// 이동 시뮬레이션, 패킷 전송
	MoveSimulate(packet);
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
	
	// 시야 안에 있는 Object 목록
	Vector<GameObjectRef> objs;
	WorldMgr.GetVisibleObjectsInSectors(GetWorldPosition(), objs, false);

	// 시야 안에 있는 타 플레이어들에게 입장 알림
	{
		Protocol::SC_OBJECT_SPAWN spawnPkt;

		// 오브젝트 정보 만들기
		MakeObjectInfo(objInfo, _playerType);
		spawnPkt.mutable_objectinfo()->CopyFrom(objInfo);

		// 타 플레이어에게 현재 캐릭터 스폰 알림
		WorldMgr.SendBroadcast(objs, spawnPkt, PacketID::SC_OBJECT_SPAWN);
	}

	// 현재 접속한 플레이어에게 시야 안에 있는 타 플레이어 목록 알림
	for (const auto& obj : objs)
	{
		if (obj->GetObjectId() == _objectId)
		{
			continue;
		}

		// 시야목록 Object 추가
		_visibleObjects.insert(obj->GetObjectId());

		// 패킷전송
		if (obj->IsPlayer())
		{
			const PlayerRef& player = std::static_pointer_cast<Player>(obj);

			Protocol::SC_OBJECT_SPAWN spawnPkt;
			Protocol::ObjectInfo otherObjInfo;

			// 타 오브젝트 정보 만들기
			player->MakeObjectInfo(otherObjInfo, player->GetPlayerType());
			spawnPkt.mutable_objectinfo()->CopyFrom(otherObjInfo);

			// 현재 캐릭터에게 타 플레이어 존재 알림
			PacketHandler::SendPacket(_session, spawnPkt, PacketID::SC_OBJECT_SPAWN);
		}
	}
	_isEntered.store(true);
}

// 시야내의 ObjectId 업데이트
void Player::UpdateVision(Vector<GameObjectRef>& currentVisible)
{
	if (_isEntered.load() == false)
	{
		return;
	}

	HashSet<ObjectId> newVisible;

	// 시야 내 새로 들어온 Object 스폰
	for (const GameObjectRef& obj : currentVisible)
	{
		const ObjectId objectId = obj->GetObjectId();
		if (objectId == _objectId)
		{
			continue;
		}

		newVisible.insert(objectId);

		if (_visibleObjects.find(objectId) == _visibleObjects.end())
		{
			// 스폰 패킷 전송
			Protocol::SC_OBJECT_SPAWN spawnPkt;
			Protocol::ObjectInfo objInfo;
			obj->MakeObjectInfo(objInfo);
			spawnPkt.mutable_objectinfo()->CopyFrom(objInfo);

			PacketHandler::SendPacket(_session, spawnPkt, PacketID::SC_OBJECT_SPAWN);
		}
	}

	// 시야 내 제거된 Object 디스폰
	for (const ObjectId& objId : _visibleObjects)
	{
		if (newVisible.find(objId) == newVisible.end())
		{
			// 디스폰 패킷 전송
			Protocol::SC_OBJECT_DESPAWN despawnPkt;
			despawnPkt.set_objectid(objId.GetId());

			PacketHandler::SendPacket(_session, despawnPkt, PacketID::SC_OBJECT_DESPAWN);
		}
	}

	// 시야 업데이트
	_visibleObjects = std::move(newVisible);
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
	WorldMgr.AddObjectToSector(shared_from_this(), zoneType, gridIndex);

	// 플레이어 zone, grid 업데이트
	SetZoneGridIndex(zoneType, gridIndex);
}

// 이동 시뮬레이션
void Player::MoveSimulate(const Protocol::CS_PLAYER_MOVE& packet)
{
	Protocol::PosInfo posInfo = packet.posinfo();

	// 캐릭터 이동속도
	int32 moveSpeed = _statInfo.movespeed();

	// 이동거리 확인
	float distanceX = posInfo.x() - _pos.x();
	float distanceY = posInfo.y() - _pos.y();
	float distance = ::sqrt(distanceX * distanceX + distanceY * distanceY);

	// 허용범위 내에 있는지
	// Player의 이동속도로 해당 시간내에 정말 이동가능한 거리인지 검증
	int32 halfRtt = _session->GetRttAvg() / 2;
	float halfRttSec = halfRtt / 1000.f;
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(NOW - _lastMoveTime).count();
	elapsedTime += halfRttSec;

	// 허용범위를 넘어섰다면 강제 보정, 패킷전송
	// - 과거 위치로 이동시킴
	float allowedDistance = moveSpeed * elapsedTime;
	if (distance > allowedDistance * MOVE_ALLOW_RANGE)
	{
		LOG_ERROR("캐릭터 이동 오차범위 초과, Distance: " + std::to_string(distance) + 
			" SessionId: " + std::to_string(_session->GetSessionID().GetID()));
		
		// 이전의 좌표(서버좌표)로 전송시킨다
		Protocol::SC_PLAYER_MOVE movePkt;
		movePkt.set_objectid(_objectId.GetId());
		movePkt.mutable_posinfo()->CopyFrom(_pos);
		WorldMgr.SendUnicast(_session, movePkt, PacketID::SC_PLAYER_MOVE);
		
		return;
	}
	// 위치 업데이트
	SetWorldPosition(packet.posinfo());
	// RTT 시간 이후의 예상좌표로 패킷 전송
	PredicateMove(packet, halfRttSec);
	// 이동 시간 업데이트
	_lastMoveTime = NOW;
}

// 예측 이동
void Player::PredicateMove(const Protocol::CS_PLAYER_MOVE& packet, const float halfRttSec)
{
	Protocol::PosInfo posInfo = packet.posinfo();
	Protocol::PosMoveVector moveVector = packet.movevector();
	
	// 예측 시간, 거리
	const float predicateDistance = _statInfo.movespeed() * halfRttSec;
	
	// 예측 좌표
	Protocol::PosInfo predicatePos;
	predicatePos.set_x(posInfo.x() + moveVector.x() * predicateDistance);
	predicatePos.set_y(posInfo.y() + moveVector.y() * predicateDistance);
	predicatePos.set_z(posInfo.z());
	predicatePos.set_yaw(posInfo.yaw());

	// 이동패킷 전송
	Protocol::SC_PLAYER_MOVE movePkt;
	movePkt.set_objectid(_objectId.GetId());
	movePkt.mutable_posinfo()->CopyFrom(predicatePos);
	WorldMgr.SendBroadcastToVisiblePlayers(shared_from_this(), movePkt, PacketID::SC_PLAYER_MOVE);
}

// playerType(직업) 정보 가져오기
Protocol::PlayerType Player::GetPlayerType() const
{
	return _playerType;
}
