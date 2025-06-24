#pragma once
#include <rapidjson/document.h>

/*-------------------------------------------------------
				BaseJsonLoader

- RapidJSON 사용 - https://rapidjson.org/
- .json 파일 읽고 파싱
--------------------------------------------------------*/

class BaseJsonLoader
{
protected:
	// 파일 읽기
	static void ReadFileToString(const std::string& filePath, std::string& outBuffer);
	// JSON 파싱 기본 검증
	static bool ParseJSONDocument(const std::string& jsonContent, rapidjson::Document& outDocument);
};

struct StringField
{
	const char* key;
	std::function<void(const std::string&)> setter;
};

struct IntField
{
	const char* key;
	std::function<void(int32)> setter;
};
