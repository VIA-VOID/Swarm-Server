#include "pch.h"
#include "BaseStat.h"

void BaseStat::GetStatInfo(Protocol::StatInfo& outStatInfo) const
{
	outStatInfo.CopyFrom(_statInfo);
}

void BaseStat::GetStatGrowth(Protocol::StatInfo& outStatGrowth) const
{
	outStatGrowth.CopyFrom(_statGrowth);
}

void BaseStat::SetStatInfo(const Protocol::StatInfo& outStatInfo)
{
	_statInfo.CopyFrom(outStatInfo);
}

void BaseStat::SetStatGrowth(const Protocol::StatInfo& outStatGrowth)
{
	_statGrowth.CopyFrom(outStatGrowth);
}
