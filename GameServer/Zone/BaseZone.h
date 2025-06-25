#pragma once
#include "ZoneDefine.h"

/*-------------------------------------------------------
				BaseZone

- 모든 Zone의 기본 클래스
--------------------------------------------------------*/
class BaseZone : public std::enable_shared_from_this<BaseZone>
{
public:
	BaseZone(const ZoneInfo& zoneInfo, uint64 updateTime = FRAME_INTERVAL.count());
	virtual ~BaseZone();

	// Zone 업데이트, 스레드에서 실행
	virtual void ZoneUpdateWorkerThread() = 0;
	// Object 스폰
	virtual void ObjectSpawn(GameObjectRef obj) = 0;
	// Object 디스폰
	virtual void ObjectDespawn(GameObjectRef obj) = 0;

	// Getter
	ZoneType GetZoneType() const;
	const ZoneInfo& GetZoneInfo() const;

protected:
	ZoneInfo _zoneInfo;
	uint64 _updateTime;
};
