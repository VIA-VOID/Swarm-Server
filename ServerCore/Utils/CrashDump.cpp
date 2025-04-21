#include "pch.h"
#include "CrashDump.h"
#include "Utils/Utils.h"
#include "Utils/Timer.h"

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

	LOG_SYSTEM(L"CrashDump instance initialized");
}

// Dump 파일명 지정
std::wstring CrashDump::MakeDumpFileName()
{
	std::wstring dumpPath = Utils::SetFilePath() + L"\\Dumps";

	// 폴더가 없으면 생성
	::CreateDirectoryW(dumpPath.c_str(), nullptr);

	return dumpPath + L"\\dump_" + Clock::GetFormattedTime(L'_', L'_') + L".dmp";
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
