#include "pch.h"
#include "Utils.h"

/*----------------------------
		Utils
----------------------------*/

// 지정한 경로 리턴
std::wstring Utils::SetFilePath()
{
	wchar_t moduleFileName[MAX_PATH] = {};
	::GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);
	std::wstring path = moduleFileName;

#ifdef _DEBUG
	// 예: /Build/Debug/*.exe → 프로젝트 폴더로
	return GoUpDirectories(path, 3);
#else
	// 실행 파일의 디렉토리까지만
	return RemoveLastPathComponent(path);
#endif
}

// 마지막 디렉토리 구분자 이전까지 자르기
std::wstring Utils::RemoveLastPathComponent(std::wstring& path)
{
	size_t pos = path.find_last_of(L'\\');
	if (pos != std::wstring::npos)
	{
		return path.substr(0, pos);
	}
	return path;
}

// 경로를 levels 상위로 이동
std::wstring Utils::GoUpDirectories(std::wstring& path, uint8 levels)
{
	for (int i = 0; i < levels; i++)
	{
		path = RemoveLastPathComponent(path);
	}
	return path;
}
