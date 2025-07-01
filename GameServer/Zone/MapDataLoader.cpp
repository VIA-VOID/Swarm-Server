#include "pch.h"
#include "MapDataLoader.h"
#include <rapidjson/error/en.h>

// JSON 파일 로드 및 파싱
bool MapDataLoader::LoadMapData(const std::string& filePath, MapData& outData)
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
	
	// JSON 맵 데이터 파싱
	ParseMapData(document, outData);
	
	LOG_SYSTEM("맵 데이터 json 로드 완료: " + outData.mapName);
	return true;
}

// JSON 파싱
void MapDataLoader::ParseMapData(const rapidjson::Document& document, MapData& outData)
{
	std::vector<StringField> stringFields = 
	{
		{ "mapName",	[&outData](const std::string& value) { outData.mapName = value; } }
	};

	std::vector<IntField> intFields =
	{
		{ "gridSize",	[&outData](int32 value) { outData.gridSize = value; } },
		{ "gridX",		[&outData](int32 value) { outData.gridX = value; } },
		{ "gridY",		[&outData](int32 value) { outData.gridY = value; } },
		{ "worldMinX",	[&outData](int32 value) { outData.worldMinX = value; } },
		{ "worldMinY",	[&outData](int32 value) { outData.worldMinY = value; } },
		{ "worldMaxX",	[&outData](int32 value) { outData.worldMaxX = value; } },
		{ "worldMaxY",	[&outData](int32 value) { outData.worldMaxY = value; } },
		{ "totalCells",	[&outData](int32 value) { outData.totalCells = value; } },
	};

	for (const auto& field : stringFields)
	{
		if (document.HasMember(field.key) && document[field.key].IsString())
		{
			field.setter(document[field.key].GetString());
		}
	}

	for (const auto& field : intFields)
	{
		if (document.HasMember(field.key) && document[field.key].IsInt())
		{
			field.setter(document[field.key].GetInt());
		}
	}

	// Zone 정보 파싱
	if (document.HasMember("zones") && document["zones"].IsArray())
	{
		if (ParseZones(document["zones"], outData.gridSize, outData.zones) == false)
		{
			LOG_ERROR("zones 파싱 실패");
			return;
		}
	}
	// 맵 그리드 파싱
	if (document.HasMember("mapData") && document["mapData"].IsArray())
	{
		if (ParseMapGrid(document["mapData"], outData.mapGrid, outData.gridX, outData.gridY) == false)
		{
			LOG_ERROR("mapData 파싱 실패");
			return;
		}
	}
}

// Zone 정보 파싱
bool MapDataLoader::ParseZones(const rapidjson::Value& zonesArray, int32 gridSize, Vector<ZoneInfo>& outZones)
{
	if (zonesArray.IsArray() == false)
	{
		return false;
	}
	outZones.reserve(zonesArray.Size());

	for (rapidjson::SizeType i = 0; i < zonesArray.Size(); i++)
	{
		const rapidjson::Value& zoneObj = zonesArray[i];
		if (zoneObj.IsObject() == false)
		{
			continue;
		}
		ZoneInfo zone;
		
		// gridSize
		zone.gridSize = gridSize;

		// zoneType
		if (zoneObj.HasMember("zoneType") && zoneObj["zoneType"].IsInt())
		{
			zone.zoneType = static_cast<ZoneType>(zoneObj["zoneType"].GetInt());
		}
		// worldPos 파싱
		if (zoneObj.HasMember("worldPos") && zoneObj["worldPos"].IsObject())
		{
			if (ParseZonePosition(zoneObj["worldPos"], zone.worldPos) == false)
			{
				LOG_ERROR("Zone " + zone.zoneName + " worldPos 파싱 실패");
				continue;
			}
		}
		// zoneName
		if (zoneObj.HasMember("zoneName") && zoneObj["zoneName"].IsString())
		{
			zone.zoneName = zoneObj["zoneName"].GetString();
		}
		outZones.push_back(zone);
	}
	return outZones.empty() == false;
}

// MapData 좌표 파싱
bool MapDataLoader::ParseMapGrid(const rapidjson::Value& mapDataArray, Vector<Vector<bool>>& outGrid, int32 gridX, int32 gridY)
{
	if (mapDataArray.IsArray() == false)
	{
		return false;
	}
	outGrid.resize(gridY);

	// 배열 크기 확인
	size_t mapDataSize = mapDataArray.Size();
	if (static_cast<int32>(mapDataSize) != gridY)
	{
		LOG_WARNING("맵 데이터 행 수가 다름: " + std::to_string(mapDataSize) + " != " + std::to_string(gridY));
	}

	// 각 행 파싱
	for (rapidjson::SizeType row = 0; row < mapDataSize && row < static_cast<rapidjson::SizeType>(gridY); row++)
	{
		const rapidjson::Value& rowArray = mapDataArray[row];
		if (rowArray.IsArray() == false)
		{
			LOG_WARNING("맵 데이터 " + std::to_string(row) + "행이 배열이 아님");
			continue;
		}

		outGrid[row].resize(gridX);

		// 각 열 파싱
		for (rapidjson::SizeType col = 0; col < rowArray.Size() && col < static_cast<rapidjson::SizeType>(gridX); col++)
		{
			const rapidjson::Value& cellValue = rowArray[col];
			outGrid[row][col] = cellValue.GetInt();
		}
	}

	return true;
}

// Zone 위치 정보 파싱
bool MapDataLoader::ParseZonePosition(const rapidjson::Value& posObj, ZonePos& outPos)
{
	if (posObj.IsObject() == false)
	{
		return false;
	}

	std::vector<IntField> intFields =
	{
		{ "minX",	[&outPos](int32 value) { outPos.minX = value; } },
		{ "minY",	[&outPos](int32 value) { outPos.minY = value; } },
		{ "maxX",	[&outPos](int32 value) { outPos.maxX = value; } },
		{ "maxY",	[&outPos](int32 value) { outPos.maxY = value; } },
	};

	for (const auto& field : intFields)
	{
		if (posObj.HasMember(field.key) && posObj[field.key].IsInt())
		{
			field.setter(posObj[field.key].GetInt());
		}
	}

	return true;
}
