#pragma once
#include "BaseZone.h"

class BossZone : public BaseZone
{
public:
	BossZone(const ZoneInfo& zoneInfo);
	virtual ~BossZone();

	void ZoneUpdate() override;
};

