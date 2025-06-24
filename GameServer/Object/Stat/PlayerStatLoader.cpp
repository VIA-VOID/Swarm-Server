#include "pch.h"
#include "PlayerStatLoader.h"

// JSON 파일 로드 및 PlayerStat 파싱
bool PlayerStatLoader::LoadPlayerStat(const std::string& filePath, PlayerStatMap& outPlayerStats)
{
	// 파일 읽기
	std::string jsonContent;
	ReadFileToString(filePath, jsonContent);

	if (jsonContent.empty())
	{
		LOG_ERROR(filePath + " 읽기 실패: " + filePath);
		return false;
	}

	// JSON 파싱
	rapidjson::Document document;
	if (ParseJSONDocument(jsonContent, document) == false)
	{
		LOG_ERROR("ParseJSONDocument 실패: " + filePath);
		return false;
	}

	// JSON PlayerStat 데이터 파싱
	ParsePlayerStat(document, outPlayerStats);

	LOG_SYSTEM("플레이어 스텟 json 로드 완료");
	return true;
}

// PlayerStat 파싱
bool PlayerStatLoader::ParsePlayerStat(const rapidjson::Document& document, PlayerStatMap& outPlayerStats)
{
	const rapidjson::Value& playerTypesArray = document["playerTypes"];

	// 각 플레이어 타입 파싱
	for (rapidjson::SizeType i = 0; i < playerTypesArray.Size(); i++)
	{
		const rapidjson::Value& playerObj = playerTypesArray[i];

		// playerType 숫자 읽기
		int32 playerTypeNum = playerObj["playerType"].GetInt();
		Protocol::PlayerType playerType = static_cast<Protocol::PlayerType>(playerTypeNum);

		// 기본 스텟 파싱
		BaseStat baseStat;
		Protocol::StatInfo statInfo;
		ParseStatInfo(playerObj["baseStat"], statInfo);
		baseStat.SetStatInfo(statInfo);

		// 성장 스텟 파싱
		Protocol::StatInfo statGrowth;
		ParseStatGrowth(playerObj["growth"], statGrowth);
		baseStat.SetStatGrowth(statGrowth);

		// Map 삽입
		outPlayerStats.insert({playerType, baseStat});
	}

	return outPlayerStats.empty() == false;
}
