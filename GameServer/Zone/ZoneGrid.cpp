#include "pch.h"
#include "ZoneGrid.h"

ZoneGrid::ZoneGrid(const ZoneInfo& zoneInfo)
	: worldPos(zoneInfo.worldPos)
{
	int32 zoneX = worldPos.maxX - worldPos.minX;
	int32 zoneY = worldPos.maxY - worldPos.minY;

	// 최대 그리드 크기
	int32 gridX = zoneX / zoneInfo.gridSize + 1;
	int32 gridY = zoneY / zoneInfo.gridSize + 1;

	// GridCell 크기에 맞게 리사이징
	grid.resize(gridY);
	for (int32 y = 0; y < gridY; y++)
	{
		grid[y].resize(gridX);
	}
}

// Zone 내 그리드 좌표 유효성 검사
bool ZoneGrid::IsValidGridIndex(const GridIndex& gridIndex)
{
	int32 maxX = static_cast<int32>(grid[0].size());
	int32 maxY = static_cast<int32>(grid.size());

	return (gridIndex.x >= 0 && gridIndex.x < maxX 
		&& gridIndex.y >= 0 && gridIndex.y < maxY);
}
