#include "pch.h"
#include "CrashDump.h"
#include <Dbghelp.h>
#include <Shlwapi.h>

// 참고 https://learn.microsoft.com/ko-kr/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump

/*----------------------------
		CrashDump
----------------------------*/
void CrashDump::Init()
{
	// 프로세스에서 처리되지 않은 예외가 발생했을 때 호출할 사용자 정의 함수를 등록
	::SetUnhandledExceptionFilter(ExceptionFilter);
}

// Dump 파일명 지정
std::wstring CrashDump::MakeDumpFileName()
{
	wchar_t moduleFileName[MAX_PATH] = {};
	::GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);
	std::wstring dumpPath = moduleFileName;

#ifdef _DEBUG
	// 실행파일 위치에서 3번 상위 폴더로 이동(프로젝트 폴더로)
	// 현재 세팅으로 프로젝트에서 실행파일이 저장되는곳은
	// $ProjectDir/Build/$Configuration/*.exe
	for (int i = 0; i < 3; i++)
	{
		// 마지막 '\'를 찾아 실행 파일 이름 제거, 디렉토리 경로만 남김
		size_t pos = dumpPath.find_last_of(L'\\');
		if (pos != std::wstring::npos)
		{
			dumpPath = dumpPath.substr(0, pos);
		}
	}
#else
	size_t pos = dumpPath.find_last_of(L'\\');
	if (pos != std::wstring::npos)
	{
		dumpPath = dumpPath.substr(0, pos);
	}
#endif

	// Dump 폴더 붙이기
	dumpPath += L"\\Dump";

	// 폴더가 없으면 생성
	::CreateDirectoryW(dumpPath.c_str(), nullptr);

	SYSTEMTIME time;
	::GetLocalTime(&time);

	WCHAR fileName[MAX_PATH];
	::wsprintf(
		fileName, L"Dump_%d_%02d_%02d_%02d%02d%02d.dmp",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond
	);

	return dumpPath + L"\\" + fileName;
}

// dump 파일 생성
LONG WINAPI CrashDump::ExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
	std::wstring dumpFileName = MakeDumpFileName();

	HANDLE hFile = ::CreateFile(
		dumpFileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo = {};
		dumpInfo.ThreadId = ::GetCurrentThreadId();
		dumpInfo.ExceptionPointers = pExceptionInfo;
		dumpInfo.ClientPointers = TRUE;

		::MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithFullMemory,
			&dumpInfo,
			nullptr,
			nullptr
		);

		::CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
