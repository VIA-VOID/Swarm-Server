#pragma once
#include "BaseStatLoader.h"

using PlayerStatMap = HashMap<Protocol::PlayerType, BaseStat>;

/*-------------------------------------------------------
				PlayerStatLoader

- 플레이어 스텟 데이터 로드
--------------------------------------------------------*/
class PlayerStatLoader : public BaseStatLoader
{
public:
	// JSON 파일 로드 PlayerStat 파싱
	static bool LoadPlayerStat(const std::string& filePath, PlayerStatMap& outPlayerStats);
	
private:
	// PlayerStat 파싱
	static bool ParsePlayerStat(const rapidjson::Document& document, PlayerStatMap& outPlayerStats);
};
