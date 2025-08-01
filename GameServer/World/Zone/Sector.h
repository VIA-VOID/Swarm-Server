#pragma once

// Sector 고유 ID
class SectorId
{
public:
	SectorId(int32 sectorX, int32 sectorY)
	{
		_id = (static_cast<uint64>(sectorX) << 32) | static_cast<uint64>(sectorY);
	}
	SectorId(const GridIndex& gridIndex)
	{
		int32 sectorX = gridIndex.x / SECTOR_SIZE;
		int32 sectorY = gridIndex.y / SECTOR_SIZE;
		_id = (static_cast<uint64>(sectorX) << 32) | static_cast<uint64>(sectorY);
	}
	uint64 GetId() const
	{
		return _id;
	}
	// 섹터 좌표 추출
	int32 GetSectorX() const { return static_cast<int32>(_id >> 32); }
	int32 GetSectorY() const { return static_cast<int32>(_id & 0xFFFFFFFF); }
	// 연산자 오버로딩
	bool operator==(const SectorId& other) const { return _id == other._id; }
	bool operator!=(const SectorId& other) const { return _id != other._id; }
	bool operator<(const SectorId& other) const { return _id < other._id; }

private:
	uint64 _id;
};

template<>
struct std::hash<SectorId>
{
	uint64 operator()(const SectorId& sectorId) const noexcept
	{
		return sectorId.GetId();
	}
};

struct ForceDeleteObject
{
	ObjectId objectId;
	ZoneType zoneType;
	GridIndex gridIndex;
};

/*--------------------------------------------------------
					Sector

- WorldManager에서 ZoneType과 매핑하여 사용
- sector별 공간, Lock 분리
--------------------------------------------------------*/
using ObjectSector = HashMap<ObjectId, GameObjectRef>;

class Sector
{
public:
	Sector(const ZoneInfo& zoneInfo);

public:
	USE_LOCK;

	ZoneInfo zoneInfo;
	HashMap<SectorId, ObjectSector> sectors;
};
