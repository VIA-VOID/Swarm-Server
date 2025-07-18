#pragma once
#include "BaseZone.h"

class TownZone : public BaseZone
{
public:
	TownZone(const ZoneInfo& zoneInfo);
	virtual ~TownZone();
	
	void ZoneUpdate() override;

	// 랜덤 스폰 위치 가져오기
	Vector3d GetRandomSpawnPosition();
};
