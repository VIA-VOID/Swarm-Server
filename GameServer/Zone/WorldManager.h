#pragma once
#include "ZoneDefine.h"
#include "BaseZone.h"
#include "Sector.h"
#include "Object/Player/Player.h"

/*--------------------------------------------------------
					WorldManager

- 월드 맵 관리
- ZoneType, Sector로 나누어진 맵 데이터(좌표) 관리
--------------------------------------------------------*/
class WorldManager : public Singleton<WorldManager>
{
public:
	void Init() override;
	void Shutdown() override;

	// Sector 주기적 업데이트
	void SectorUpdateWorkerThread();
	// 시야내 GameObject 목록 업데이트
	void UpdateVisible();
	// Sector별 오브젝트 추가
	void AddObjectToSector(GameObjectRef obj, const ZoneType zoneType, const GridIndex& gridIndex);
	// 오브젝트 제거
	void RemoveObjectToSector(const ObjectId objId, const ZoneType zoneType, const GridIndex& gridIndex);
	// 시야 내의 GameObject 목록 가져오기
	// 여러 Zone, Sector 검색
	void GetVisibleObjectsInSectors(const Vector3d& position, Vector<GameObjectRef>& outObjects, bool onlyPlayer = true);
	// 월드좌표가 유효한지 판별
	bool IsValidWorldPosition(const Protocol::PosInfo& posInfo) const;
	// 이동가능한 지점인지 판별
	// - 월드좌표로 인자를 받고, 내부에서 그리드 좌표로 변환해서 판별
	bool CanGo(float worldX, float worldY);
	// zone 가져오기
	BaseZone* FindZone(const ZoneType zoneType);
	// 월드 좌표로 Zone 타입 찾기
	ZoneType GetZoneByPosition(const Vector3d& position) const;
	// ZonePos 가져오기
	ZonePos GetZonePosByType(const ZoneType zoneType);
	// Sector 가져오기
	Sector* FindSector(const ZoneType zoneType);

	// 단일대상 sendPacket
	template <typename T>
	void SendUnicast(SessionRef targetSession, const T& message, const PacketID pktId);
	// 복수대상 sendPacket
	template <typename T>
	void SendBroadcast(const Vector<GameObjectRef>& objects, const T& message, const PacketID pktId, const ObjectId exceptId = ObjectId(-1));
	// 시야 내 플레이어들에게 패킷 전송
	template <typename T>
	void SendBroadcastToVisiblePlayers(const GameObjectRef targetPlayer, const T& message, const PacketID pktId);

private:
	// Zone 초기화
	void InitZones();
	// Sector 초기화
	void InitSectors();
	// 그리드좌표 유효성 검사
	bool IsVaildGridIndex(const GridIndex& gridIndex) const;
	// 시야 내의 GameObject 목록 가져오기
	// 단일 Zone 검색
	void GetVisibleObjectsInSector(ZoneType zoneType, const Vector3d& position, Vector<GameObjectRef>& outObjects, bool onlyPlayer = true);
	// 시야 범위에 포함되는 Zone 목록 가져오기
	void GetVisibleZones(const Vector3d& position, Vector<ZoneType>& outZoneTypes);
	// 시야내의 SectorId 가져오기
	void GetVisibleSectorIds(const GridIndex& gridIndex, Vector<SectorId>& outSectorIds);
	// 시야 범위에 Zone이 속하는지 여부
	bool IsInRange(const Vector3d& position, const ZonePos& worldPos, int32 gridSize);
	// 그리드 범위 체크
	bool IsInGridRange(const GridIndex& gridIndex, const GridIndex& target);
	// ZoneType으로 해당 Zone의 좌표범위 가져오기
	ZonePos GetZonePositionByType(const ZoneType zoneType);
	// empty sector 정리
	void ClearAllEmptySector();

private:
	USE_LOCK;
	// 맵 데이터
	MapData _mapData;
	// Zone별 Sector
	HashMap<ZoneType, SectorURef> _zoneSectors;
	// Zone 관리
	HashMap<ZoneType, BaseZoneURef> _zones;
};

// 단일대상 sendPacket
template <typename T>
inline void WorldManager::SendUnicast(SessionRef targetSession, const T& message, const PacketID pktId)
{
	if (targetSession == nullptr || targetSession->IsClosed())
	{
		return;
	}
	PacketHandler::SendPacket(targetSession, message, pktId);
}

// 복수대상 sendPacket
template <typename T>
inline void WorldManager::SendBroadcast(const Vector<GameObjectRef>& objects, const T& message, const PacketID pktId, const ObjectId exceptId /*= ObjectId(-1)*/)
{
	for (const auto& obj : objects)
	{
		const PlayerRef& player = std::static_pointer_cast<Player>(obj);
		if (player->GetObjectId() == exceptId)
		{
			continue;
		}
		SendUnicast(player->GetSession(), message, pktId);
	}
}

// 시야 내 플레이어들에게 패킷 전송
template <typename T>
inline void WorldManager::SendBroadcastToVisiblePlayers(const GameObjectRef targetPlayer, const T& message, const PacketID pktId)
{
	Vector<GameObjectRef> players;
	GetVisibleObjectsInSector(targetPlayer->GetCurrentZone(), targetPlayer->GetWorldPosition(), players);

	SendBroadcast(players, message, pktId, targetPlayer->GetObjectId());
}
