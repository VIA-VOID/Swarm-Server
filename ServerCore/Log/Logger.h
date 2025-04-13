#pragma once

enum class LogType : uint8
{
	Info,
	System,
	Error,
};

struct LogMessage
{
	LogType _type;
	std::wstring _message;
	std::wstring _timeStamp;
	std::thread::id _threadId;
};

/*--------------------------------------------------------
					Logger
--------------------------------------------------------*/
class Logger
{
public:
	// 로그찍기
	void Log(LogMessage& msg);

private:
	// LogType에 따라 색 지정
	void PrintColor(const LogType& level);
	// 색 초기화
	void ResetColor();
};
