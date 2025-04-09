#pragma once

/*----------------------------
		CrashDump
----------------------------*/
class CrashDump : public Singleton<CrashDump>
{
public:
	// CrashDump 활성화
	CrashDump();
	// dump 파일 생성
	static LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo);

private:
	// Dump 파일명 지정
	static std::wstring MakeDumpFileName();
};
