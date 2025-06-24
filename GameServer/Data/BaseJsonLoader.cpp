#include "pch.h"
#include "BaseJsonLoader.h"
#include <rapidjson/error/en.h>

// 파일 읽기
void BaseJsonLoader::ReadFileToString(const std::string& filePath, std::string& outBuffer)
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

// JSON 파싱 기본 검증
bool BaseJsonLoader::ParseJSONDocument(const std::string& jsonContent, rapidjson::Document& outDocument)
{
	rapidjson::ParseResult result = outDocument.Parse(jsonContent.c_str());
	if (result == nullptr)
	{
		LOG_ERROR("JSON 파싱 실패: " + std::string(rapidjson::GetParseError_En(result.Code())) +
			" (오프셋: " + std::to_string(result.Offset()) + ")");
		return false;
	}
	return true;
}
