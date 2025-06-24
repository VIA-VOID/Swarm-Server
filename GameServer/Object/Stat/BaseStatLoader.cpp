#include "pch.h"
#include "BaseStatLoader.h"

// 기본 스텟 파싱
void BaseStatLoader::ParseStatInfo(const rapidjson::Value& baseStatObj, Protocol::StatInfo& outStatInfo)
{
	Vector<IntField> fields =
	{
		{ "level",		[&outStatInfo](int32 value) { outStatInfo.set_level(value); } },
		{ "hp",			[&outStatInfo](int32 value) { outStatInfo.set_hp(value); } },
		{ "maxHp",		[&outStatInfo](int32 value) { outStatInfo.set_maxhp(value); } },
		{ "mp",			[&outStatInfo](int32 value) { outStatInfo.set_mp(value); } },
		{ "maxMp",		[&outStatInfo](int32 value) { outStatInfo.set_maxmp(value); } },
		{ "attack",		[&outStatInfo](int32 value) { outStatInfo.set_attack(value); } },
		{ "defense",	[&outStatInfo](int32 value) { outStatInfo.set_defense(value); } },
		{ "moveSpeed",	[&outStatInfo](int32 value) { outStatInfo.set_movespeed(value); } },
		{ "attackSpeed",[&outStatInfo](int32 value) { outStatInfo.set_attackspeed(value); } },
	};

	for (const auto& field : fields)
	{
		if (baseStatObj.HasMember(field.key) && baseStatObj[field.key].IsInt())
		{
			field.setter(baseStatObj[field.key].GetInt());
		}
	}
}

// 성장 스텟 파싱
void BaseStatLoader::ParseStatGrowth(const rapidjson::Value& growthObj, Protocol::StatInfo& outStatGrowth)
{
	Vector<IntField> fields =
	{
		{ "hp",			[&outStatGrowth](int32 value) { outStatGrowth.set_hp(value); } },
		{ "maxHp",		[&outStatGrowth](int32 value) { outStatGrowth.set_maxhp(value); } },
		{ "mp",			[&outStatGrowth](int32 value) { outStatGrowth.set_mp(value); } },
		{ "maxMp",		[&outStatGrowth](int32 value) { outStatGrowth.set_maxmp(value); } },
		{ "attack",		[&outStatGrowth](int32 value) { outStatGrowth.set_attack(value); } },
		{ "defense",	[&outStatGrowth](int32 value) { outStatGrowth.set_defense(value); } },
	};

	for (const auto& field : fields)
	{
		if (growthObj.HasMember(field.key) && growthObj[field.key].IsInt())
		{
			field.setter(growthObj[field.key].GetInt());
		}
	}
}
