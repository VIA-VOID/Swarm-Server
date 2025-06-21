#pragma once
#include "ZoneDefine.h"
#include <rapidjson/document.h>

/*-------------------------------------------------------
				MapDataLoader

- RapidJSON 사용 - https://rapidjson.org/
- 맵 (좌표)데이터를 읽고 파싱
--------------------------------------------------------*/

class MapDataLoader
{
public:
	// JSON 파일 로드 및 파싱
	static bool LoadFromFile(const std::string& filePath, MapData& outData);

private:
	// 파일 읽기
	static void ReadFileToString(const std::string& filePath, std::string& outBuffer);
	// JSON 파싱
	static bool ParseJSON(const std::string& jsonContent, MapData& outData);
	// Zone 정보 파싱
	static bool ParseZones(const rapidjson::Value& zonesArray, Vector<ZoneInfo>& outZones);
	// MapData 좌표 파싱
	static bool ParseMapGrid(const rapidjson::Value& mapDataArray, Vector<Vector<int32>>& outGrid, int32 gridX, int32 gridY);
	// Zone 위치 정보 파싱
	static bool ParseZonePosition(const rapidjson::Value& posObj, ZonePos& outPos);
};
