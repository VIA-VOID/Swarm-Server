#pragma once

/*----------------------------
		CrashDump
----------------------------*/
class CrashDump
{
public:
	// CrashDump 활성화
	static void Init();
	// dump 파일 생성
	static LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo);

private:
	// Dump 파일명 지정
	static std::wstring MakeDumpFileName();
};
