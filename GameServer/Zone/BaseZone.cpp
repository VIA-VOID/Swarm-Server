#include "pch.h"
#include "BaseZone.h"

BaseZone::BaseZone(const ZoneInfo& zoneInfo, uint64 updateTime/* = MAP_TICK_INTERVAL.count()*/)
	: _zoneInfo(zoneInfo), _updateTime(updateTime)
{
}

BaseZone::~BaseZone()
{
}

// Zone World 좌표 가져오기
ZonePos BaseZone::GetWorldPosition() const
{
	return _zoneInfo.worldPos;
}
