#include "pch.h"
#include "BaseZone.h"

BaseZone::BaseZone(const ZoneInfo& zoneInfo, uint64 updateTime/* = MAP_TICK_INTERVAL.count()*/)
	: _zoneInfo(zoneInfo), _updateTime(updateTime)
{
}

BaseZone::~BaseZone()
{
}

ZoneType BaseZone::GetZoneType() const
{
	return _zoneInfo.zoneType;
}

const ZoneInfo& BaseZone::GetZoneInfo() const
{
	return _zoneInfo;
}
