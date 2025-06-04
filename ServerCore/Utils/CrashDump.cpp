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
}

// Dump 파일명 지정
std::string CrashDump::MakeDumpFileName()
{
	std::string dumpPath = Utils::SetFilePath() + "\\Dumps";

	// 폴더가 없으면 생성
	::CreateDirectoryA(dumpPath.c_str(), nullptr);

	return dumpPath + "\\dump_" + Clock::GetFormattedTime('_', '_') + ".dmp";
}

// dump 파일 생성
LONG WINAPI CrashDump::ExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
	std::string dumpFileName = MakeDumpFileName();

	HANDLE hFile = ::CreateFileA(
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
