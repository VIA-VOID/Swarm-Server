#pragma once
#include "Data/BaseJsonLoader.h"
#include "BaseStat.h"
#include <rapidjson/document.h>

/*-------------------------------------------------------
				BaseStatLoader

- GameObject 스탯 공용 파싱 클래스
--------------------------------------------------------*/

class BaseStatLoader : public BaseJsonLoader
{
protected:
	// 기본 스텟 파싱
	static void ParseStatInfo(const rapidjson::Value& baseStatObj, Protocol::StatInfo& outStatInfo);
	// 성장 스텟 파싱
	static void ParseStatGrowth(const rapidjson::Value& growthObj, Protocol::StatInfo& outStatGrowth);
};
