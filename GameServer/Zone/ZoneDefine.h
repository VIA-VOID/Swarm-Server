#pragma once
#include "pch/Types.h"
#include "Object/ObjectDefine.h"
#include <string>

// Zone 타입
enum class ZoneType : uint8
{
	Town = 0,
	Pvp = 1,
	Pve = 2,
	Boss = 3
};

// Zone 좌표
struct ZonePos
{
	int32 minX;
	int32 minY;
	int32 maxX;
	int32 maxY;
};

// Zone 정보
struct ZoneInfo
{
	ZoneType zoneType;
	ZonePos worldPos;
	int32 gridSize;
	std::string zoneName;
};

// 맵 데이터
struct MapData
{
	std::string mapName;
	int32 gridSize;
	int32 gridX;
	int32 gridY;
	int32 worldMinX;
	int32 worldMinY;
	int32 worldMaxX;
	int32 worldMaxY;
	int32 totalCells;
	Vector<ZoneInfo> zones;
	Vector<Vector<bool>> mapGrid;
};

// 그리드 인덱스
struct GridIndex
{
	int32 x;
	int32 y;

	GridIndex() = default;
	GridIndex(int32 _x, int32 _y) 
		: x(_x), y(_y) 
	{
	}

	bool operator==(const GridIndex& other) const 
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(const GridIndex& other) const 
	{
		return !(*this == other);
	}
};
