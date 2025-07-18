#pragma once
#include "BaseZone.h"

class PvpZone : public BaseZone
{
public:
	PvpZone(const ZoneInfo& zoneInfo);
	virtual ~PvpZone();

	void ZoneUpdate() override;

};

