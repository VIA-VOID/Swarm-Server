#pragma once
#include <fstream>

/*--------------------------------------------------------
					LogDispatcher

- 로그파일 저장
- [debug 모드] console에 로그 찍기
--------------------------------------------------------*/
class LogDispatcher : public Singleton<LogDispatcher>
{
public:
	// 파일생성, 초기화
	void Init();
	// dispatcher 종료
	void Shutdown();
	// 로그 쌓기
	void PushLog(const LogType& type, const std::wstring& msg, const char* fnName);
	// 스레드 일감
	void ProcessThread();

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
	bool _writing = false;
	std::wofstream _file;
	EventLockQueue<std::pair<LogType, std::wstring>> _queue;
	Vector<std::wstring> _buffer;
	std::chrono::steady_clock::time_point _lastFlushTime;
};
