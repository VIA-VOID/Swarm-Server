#pragma once
#include "Pch/Types.h"
#include <string>
#include <sstream>
#include <thread>
#include <iomanip>

enum class LogType : uint8
{
	Info,
	System,
	Warning,
	Error,
};

struct LogMessage
{
	LogType _type = LogType::Info;
	std::wstring _message;
	std::wstring _timeStamp;
	ThreadId _threadId;
	const char* functionName;

	std::wstring MakeLogWString()
	{
		std::wstringstream ws;
		ws << _timeStamp;

		switch (_type)
		{
		case LogType::Info:
			ws << L" INFO    ";
			break;
		case LogType::System:
			ws << L" SYSTEM  ";
			break;
		case LogType::Warning:
			ws << L" WARNING ";
			break;
		case LogType::Error:
			ws << L" ERROR   ";
			break;
		}

		ws << std::left << std::setw(6)
			<< _threadId << L"--- "
			<< std::left << std::setw(30)
			<< functionName
			<< L" : "
			<< _message << L"\n";
		return ws.str();
	}
};
