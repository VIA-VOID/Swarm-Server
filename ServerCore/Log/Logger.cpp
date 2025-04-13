#include "pch.h"
#include "Logger.h"

/*----------------------------
		Lock
----------------------------*/

// 로그찍기
void Logger::Log(LogMessage& msg)
{
	// LogLevel에 따라 색 지정
	PrintColor(msg._type);

	std::wstring levelStr;
	switch (msg._type)
	{
	case LogType::Info:
		levelStr = L"[INFO]   ";
		break;
	case LogType::System:
		levelStr = L"[SYSTEM] ";
		break;
	case LogType::Error:
		levelStr = L"[ERROR]  ";
		break;
	}

	std::wcout << levelStr << L" [" << msg._threadId << "] "
		<< msg._timeStamp << L" : "
		<< msg._message << "\n";

	ResetColor();
}

// LogType에 따라 색 지정
void Logger::PrintColor(const LogType& level)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	switch (level)
	{
	case LogType::Info:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
	case LogType::System:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
	case LogType::Error:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); break;
	}
}

// 색 초기화
void Logger::ResetColor()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
