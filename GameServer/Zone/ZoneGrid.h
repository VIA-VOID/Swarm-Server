#pragma once

/*--------------------------------------------------------
					ZoneGrid

- WorldManager에서 HashMap으로 ZoneType과 매핑하여 사용
- zone별 Grid 좌표 매핑
- zone별 Lock 분리
--------------------------------------------------------*/
class ZoneGrid
{
public:
	ZoneGrid(const ZoneInfo& zoneInfo);

	// Zone 내 그리드 좌표 유효성 검사
	bool IsValidGridIndex(const GridIndex& inGridIndex);

public:
	USE_LOCK;
	ZonePos worldPos;
	Vector<Vector<GridCell>> grid;
};
