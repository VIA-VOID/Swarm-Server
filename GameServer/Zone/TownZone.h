#pragma once
#include "BaseZone.h"

class TownZone : public BaseZone
{
public:
	TownZone(const ZoneInfo& zoneInfo);
	virtual ~TownZone();
	void ZoneUpdateWorkerThread() override;
	// Object 스폰
	void ObjectSpawn(GameObjectRef obj) override;
	// Object 디스폰
	void ObjectDespawn(GameObjectRef obj) override;
};
