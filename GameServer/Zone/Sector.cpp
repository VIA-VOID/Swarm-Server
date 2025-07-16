#include "pch.h"
#include "Sector.h"

Sector::Sector(const ZoneInfo& zoneInfo)
	: zoneInfo(zoneInfo)
{
}

// Zone 내 그리드 좌표 유효성 검사
bool Sector::IsValidGridIndex(const GridIndex& gridIndex) const
{
	ZonePos worldPos = zoneInfo.worldPos;

	int32 zoneGridX = (worldPos.maxX - worldPos.minX) / zoneInfo.gridSize;
	int32 zoneGridY = (worldPos.maxY - worldPos.minY) / zoneInfo.gridSize;
	
	return (gridIndex.x >= 0 && gridIndex.x < zoneGridX 
		&& gridIndex.y >= 0 && gridIndex.y < zoneGridY);
}
