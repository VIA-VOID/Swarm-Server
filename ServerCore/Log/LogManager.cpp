#include "pch.h"
#include "LogManager.h"
#include "Utils/Utils.h"
#include "Utils/Timer.h"
#include <codecvt>

/*----------------------------
		LogManager
----------------------------*/

// flush() 하는 조건
// 1. _buffer에 쌓아두고, _flushSize 크기 이상
constexpr uint8 _flushSize = 10;
// 2. _flushTime 이상
constexpr std::chrono::seconds _flushTime = std::chrono::seconds(2);

// 파일생성, 초기화
void LogManager::Init()
{
	_buffer.resize(32);

	CreateLogFile();
	_lastFlushTime = NOW;
}

// dispatcher 종료
void LogManager::Shutdown()
{
	// 파일 닫기
	CloseLogFile();
}

// 로그 쌓기
void LogManager::PushLog(const LogType type, const std::string& message, const char* fnName)
{
	LogMessage log(type, message, Clock::GetFormattedTime(), std::this_thread::get_id(), fnName);

	JobQ.DoAsync([this, log = std::move(log)]() {
		ProcessThread(log._type, log.ToString());
		}, JobGroups::System);
}

// 스레드 일감
void LogManager::ProcessThread(const LogType type, const std::string& message)
{
#if _DEBUG
	// console log
	PrintColor(type);
	std::cerr << message;
	ResetColor();
#endif

	// 일자 변경 확인
	if (Clock::IsNewDay())
	{
		// 일자 변경시 파일 close후 새로 생성
		CloseLogFile();
		CreateLogFile();
	}

	_buffer.push_back(message);

	// 조건에 맞게 flush
	// 1. _buffer에 쌓아두고, _flushSize 크기 이상
	// 2. _flushTime 이상
	if (_buffer.size() >= _flushSize || (NOW - _lastFlushTime) >= _flushTime)
	{
		FlushBuffer();
		_lastFlushTime = NOW;
	}
}

// 파일 생성
void LogManager::CreateLogFile()
{
	std::string dir = Utils::SetFilePath() + "\\Logs";
	::CreateDirectoryA(dir.c_str(), nullptr);

	std::string fileName = dir + "\\Serverlog_" + Clock::GetFormattedDate('_') + ".log";
	_file.open(fileName, std::ios::app);
}

// 파일 flush
void LogManager::FlushBuffer()
{
	for (const auto& log : _buffer)
	{
		_file.write(log.c_str(), log.size());
	}
	_file.flush();
	_buffer.clear();
}

// 파일 닫기
void LogManager::CloseLogFile()
{
	if (_file.is_open())
	{
		FlushBuffer();
		_file.close();
	}
}

// LogType에 따라 색 지정
void LogManager::PrintColor(const LogType& level)
{
	HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
	switch (level)
	{
	case LogType::Info:
		::SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
		break;
	case LogType::System:
		::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case LogType::Warning:
		::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case LogType::Error:
		::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	}
}

// 색 초기화
void LogManager::ResetColor()
{
	HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}
