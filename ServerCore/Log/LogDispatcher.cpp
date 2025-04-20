#include "pch.h"
#include "Utils/Utils.h"
#include "Utils/Timer.h"
#include <codecvt>

/*----------------------------
		LogDispatcher
----------------------------*/

// flush() 하는 조건
// 1. _buffer에 쌓아두고, _flushSize 크기 이상
constexpr uint8 _flushSize = 10;
// 2. _flushTime 이상
constexpr std::chrono::seconds _flushTime = std::chrono::seconds(2);

// 파일생성, 초기화
void LogDispatcher::Init()
{
	CreateLogFile();
	_writing = true;
	_lastFlushTime = NOW;

	// 스레드 할당
	THREAD_MANAGER.Push([=]
		{
			ProcessThread();
		}
	);

	LOG_SYSTEM(L"LogManager instance initialized");
}

// dispatcher 종료
void LogDispatcher::Shutdown()
{
	_writing = false;
	// 스레드 wake 유도
	_queue.Push({ LogType::System, L"" });
	// 파일 닫기
	CloseLogFile();
}

// 로그 쌓기
void LogDispatcher::PushLog(const LogType& type, const std::wstring& msg, const char* fnName)
{
	LogMessage log(type, msg, CLOCK.GetFormattedTime(), std::this_thread::get_id(), fnName);
	std::wstring logMessage = log.MakeLogWString();
	_queue.Push({ type, logMessage });
}

// 스레드 일감
void LogDispatcher::ProcessThread()
{
	while (_writing)
	{
		auto pop = _queue.Pop();

		// 기다리는동안 _writing 변경 될수도 있음
		if (_writing == false)
		{
			break;
		}

#if _DEBUG
		// console log
		PrintColor(pop.first);
		std::cout << UTILS.convertUtf8(pop.second);
		ResetColor();
#endif

		// 일자 변경 확인
		if (CLOCK.IsNewDay())
		{
			// 일자 변경시 파일 close후 새로 생성
			CloseLogFile();
			CreateLogFile();
		}

		_buffer.push_back(pop.second);

		// 조건에 맞게 flush
		// 1. _buffer에 쌓아두고, _flushSize 크기 이상
		// 2. _flushTime 이상
		if (_buffer.size() >= _flushSize || (NOW - _lastFlushTime) >= _flushTime)
		{
			FlushBuffer();
			_lastFlushTime = NOW;
		}
	}
}

// 파일 생성
void LogDispatcher::CreateLogFile()
{
	std::wstring dir = UTILS.SetFilePath() + L"\\Logs";
	::CreateDirectoryW(dir.c_str(), nullptr);

	std::wstring fileName = dir + L"\\Serverlog_" + CLOCK.GetFormattedDate(L'_') + L".log";
	_file.open(fileName, std::ios::app);

	if (_file.tellp() == 0)
	{
		// UTF-8 BOM 추가
		const char utf8BOM[] = "\xEF\xBB\xBF";
		_file.write(utf8BOM, sizeof(utf8BOM) - 1);
	}
}

// 파일 flush
void LogDispatcher::FlushBuffer()
{
	for (const auto& log : _buffer)
	{
		std::string utf8 = UTILS.convertUtf8(log);
		_file.write(utf8.c_str(), utf8.size());
	}
	_file.flush();
	_buffer.clear();
}

// 파일 닫기
void LogDispatcher::CloseLogFile()
{
	if (_file.is_open())
	{
		FlushBuffer();
		_file.close();
	}
}

// LogType에 따라 색 지정
void LogDispatcher::PrintColor(const LogType& level)
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
void LogDispatcher::ResetColor()
{
	HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}
