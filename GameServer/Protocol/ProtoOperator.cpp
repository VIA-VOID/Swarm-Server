#include "pch.h"
#include "ProtoOperator.h"

Protocol::StatInfo& operator+=(Protocol::StatInfo& statInfo, const Protocol::StatInfo& otherInfo)
{
	// moveSpeed와 attackSpeed는 더하지 않음
	statInfo.set_level(statInfo.level() + otherInfo.level());
	statInfo.set_hp(statInfo.hp() + otherInfo.hp());
	statInfo.set_maxhp(statInfo.maxhp() + otherInfo.maxhp());
	statInfo.set_mp(statInfo.mp() + otherInfo.mp());
	statInfo.set_maxmp(statInfo.maxmp() + otherInfo.maxmp());
	statInfo.set_attack(statInfo.attack() + otherInfo.attack());
	statInfo.set_defense(statInfo.defense() + otherInfo.defense());
	return statInfo;
}