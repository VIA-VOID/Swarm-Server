#include "pch.h"
#include "MapDataLoader.h"
#include <rapidjson/error/en.h>

// JSON 파일 로드 및 파싱
bool MapDataLoader::LoadFromFile(const std::string& filePath, MapData& outData)
{
	// 파일 읽기
	std::string jsonContent;
	ReadFileToString(filePath, jsonContent);

	if (jsonContent.empty())
	{
		LOG_ERROR(filePath + " 읽기 실패: " + filePath);
		return false;
	}
	// 파싱
	if (ParseJSON(jsonContent, outData) == false)
	{
		LOG_ERROR("ParseJSON 실패: " + filePath);
		return false;
	}
	LOG_SYSTEM("맵 데이터 로드 완료: " + outData.mapName);
	return true;
}

// 파일 읽기
void MapDataLoader::ReadFileToString(const std::string& filePath, std::string& outBuffer)
{
	std::ifstream file(filePath, std::ios::binary);
	if (file.is_open() == false)
	{
		return;
	}
	// 파일 크기 구하기
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0);
	
	// 이동
	std::string buffer(size, '\0');
	file.read(&buffer[0], size);
	outBuffer = std::move(buffer);
}

// JSON 파싱
bool MapDataLoader::ParseJSON(const std::string& jsonContent, MapData& outData)
{
	// RapidJSON Document 생성
	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(jsonContent.c_str());

	if (parseResult == nullptr)
	{
		LOG_ERROR("JSON 파싱 에러: " + std::string(rapidjson::GetParseError_En(parseResult.Code())) +
			" (오프셋: " + std::to_string(parseResult.Offset()) + ")");
		return false;
	}

	// 정보 추출
	if (document.HasMember("mapName") && document["mapName"].IsString())
	{
		outData.mapName = document["mapName"].GetString();
	}
	if (document.HasMember("gridSize") && document["gridSize"].IsInt())
	{
		outData.gridSize = document["gridSize"].GetInt();
	}
	if (document.HasMember("gridX") && document["gridX"].IsInt())
	{
		outData.gridX = document["gridX"].GetInt();
	}
	if (document.HasMember("gridY") && document["gridY"].IsInt())
	{
		outData.gridY = document["gridY"].GetInt();
	}
	if (document.HasMember("worldMinX") && document["worldMinX"].IsInt())
	{
		outData.worldMinX = document["worldMinX"].GetInt();
	}
	if (document.HasMember("worldMinY") && document["worldMinY"].IsInt())
	{
		outData.worldMinY = document["worldMinY"].GetInt();
	}
	if (document.HasMember("worldMaxX") && document["worldMaxX"].IsInt())
	{
		outData.worldMaxX = document["worldMaxX"].GetInt();
	}
	if (document.HasMember("worldMaxY") && document["worldMaxY"].IsInt())
	{
		outData.worldMaxY = document["worldMaxY"].GetInt();
	}
	if (document.HasMember("totalCells") && document["totalCells"].IsInt())
	{
		outData.totalCells = document["totalCells"].GetInt();
	}
	// Zone 정보 파싱
	if (document.HasMember("zones") && document["zones"].IsArray())
	{
		if (ParseZones(document["zones"], outData.zones) == false)
		{
			LOG_ERROR("zones 파싱 실패");
			return false;
		}
	}
	// 맵 그리드 파싱
	if (document.HasMember("mapData") && document["mapData"].IsArray())
	{
		if (ParseMapGrid(document["mapData"], outData.mapGrid, outData.gridX, outData.gridY) == false)
		{
			LOG_ERROR("mapData 파싱 실패");
			return false;
		}
	}
	return true;
}

// Zone 정보 파싱
bool MapDataLoader::ParseZones(const rapidjson::Value& zonesArray, Vector<ZoneInfo>& outZones)
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
		
		// zoneType
		if (zoneObj.HasMember("zoneType") && zoneObj["zoneType"].IsInt())
		{
			zone.zoneType = static_cast<ZoneType>(zoneObj["zoneType"].GetInt());
		}
		// zoneName
		if (zoneObj.HasMember("zoneName") && zoneObj["zoneName"].IsString())
		{
			zone.zoneName = zoneObj["zoneName"].GetString();
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
		// serverPos 파싱
		if (zoneObj.HasMember("serverPos") && zoneObj["serverPos"].IsObject())
		{
			if (ParseZonePosition(zoneObj["serverPos"], zone.serverPos) == false)
			{
				LOG_ERROR("Zone " + zone.zoneName + " serverPos 파싱 실패");
				continue;
			}
		}
		outZones.push_back(zone);
	}
	return outZones.empty() == false;
}

// MapData 좌표 파싱
bool MapDataLoader::ParseMapGrid(const rapidjson::Value& mapDataArray, Vector<Vector<int32>>& outGrid, int32 gridX, int32 gridY)
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
	if (posObj.HasMember("minX") && posObj["minX"].IsInt())
	{
		outPos.minX = posObj["minX"].GetInt();
	}
	if (posObj.HasMember("minY") && posObj["minY"].IsInt())
	{
		outPos.minY = posObj["minY"].GetInt();
	}
	if (posObj.HasMember("maxX") && posObj["maxX"].IsInt())
	{
		outPos.maxX = posObj["maxX"].GetInt();
	}
	if (posObj.HasMember("maxY") && posObj["maxY"].IsInt())
	{
		outPos.maxY = posObj["maxY"].GetInt();
	}

	return true;
}
