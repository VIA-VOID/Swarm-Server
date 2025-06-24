#pragma once
#include "Data/BaseJsonLoader.h"
#include "ZoneDefine.h"
#include <rapidjson/document.h>

/*-------------------------------------------------------
				MapDataLoader

- 맵 (좌표)데이터를 읽고 파싱
--------------------------------------------------------*/

class MapDataLoader : public BaseJsonLoader
{
public:
	// 맵 데이터 읽고 파싱
	static bool LoadMapData(const std::string& filePath, MapData& outData);

private:
	// JSON 파싱
	static void ParseMapData(const rapidjson::Document& document, MapData& outData);
	// Zone 정보 파싱
	static bool ParseZones(const rapidjson::Value& zonesArray, Vector<ZoneInfo>& outZones);
	// MapData 좌표 파싱
	static bool ParseMapGrid(const rapidjson::Value& mapDataArray, Vector<Vector<bool>>& outGrid, int32 gridX, int32 gridY);
	// Zone 위치 정보 파싱
	static bool ParseZonePosition(const rapidjson::Value& posObj, ZonePos& outPos);
};
