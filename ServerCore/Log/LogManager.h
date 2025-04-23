#pragma once
#include <fstream>

/*--------------------------------------------------------
					LogManager

- 로그파일 저장
- [debug 모드] console에 로그 찍기
--------------------------------------------------------*/
class LogManager : public Singleton<LogManager>
{
public:
	// 파일생성, 초기화
	void Init() override;
	// 로그 쌓기
	void PushLog(const LogType type, const std::wstring& msg, const char* fnName);
	// 스레드 일감
	void ProcessThread(const LogType type, const std::wstring message);
	// 종료
	void Shutdown() override;

private:
	// 파일 생성
	void CreateLogFile();
	// 파일 flush
	void FlushBuffer();
	// 파일 닫기
	void CloseLogFile();
	// LogType에 따라 색 지정
	void PrintColor(const LogType& type);
	// 색 초기화
	void ResetColor();

private:
	std::ofstream _file;
	Vector<std::wstring> _buffer;
	std::chrono::steady_clock::time_point _lastFlushTime;
};
