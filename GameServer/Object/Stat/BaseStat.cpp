#include "pch.h"
#include "BaseStat.h"

// 레벨업시 스텟 계산
void BaseStat::CalculateLevelUpStat(Protocol::StatInfo& currentStat) const
{
	currentStat += _statGrowth;
}

void BaseStat::SetStatInfo(const Protocol::StatInfo& statInfo)
{
	_statInfo.CopyFrom(statInfo);
}

void BaseStat::SetStatGrowth(const Protocol::StatInfo& statGrowth)
{
	_statGrowth.CopyFrom(statGrowth);
}
