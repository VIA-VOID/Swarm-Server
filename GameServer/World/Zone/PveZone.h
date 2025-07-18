#pragma once
#include "BaseZone.h"

class PveZone : public BaseZone
{
public:
	PveZone(const ZoneInfo& zoneInfo);
	virtual ~PveZone();

	void ZoneUpdate() override;

};

