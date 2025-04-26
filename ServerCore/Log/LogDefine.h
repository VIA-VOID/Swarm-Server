#pragma once
#include "Pch/Types.h"
#include "Pch/CoreTLS.h"
#include <string>
#include <sstream>
#include <thread>
#include <iomanip>
#include <iostream>

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
	WString _message;
	WString _timeStamp;
	const char* _functionName;

	LogMessage()
		: _type(LogType::Info), _message(L"\0"), _timeStamp(L"\0"), _functionName(nullptr)
	{
	}

	LogMessage(LogType type, WString msg, WString timeStamp, ThreadId id, const char* fnName)
		: _type(type), _message(msg), _timeStamp(timeStamp), _functionName(fnName)
	{
	}

	WString ToWString() const
	{
		StringStream ss;

		// 로그 타입 문자열
		WString typeStr = L"UNKNOWN";
		switch (_type)
		{
		case LogType::Info:
			typeStr = L" INFO    ";
			break;
		case LogType::System:
			typeStr = L" SYSTEM  ";
			break;
		case LogType::Warning:
			typeStr = L" WARNING ";
			break;
		case LogType::Error:
			typeStr = L" ERROR   ";
			break;
		}

		// threadName 좌측 정렬 (6자 고정 너비)
		StringStream tnStream;
		tnStream << std::left << std::setw(6) << WString(LThreadName.begin(), LThreadName.end());
		WString threadNameStr = tnStream.str();

		// function name 좌측 정렬 (30자 고정 너비)
		StringStream fnStream;
		fnStream << std::left << std::setw(30) << WString(_functionName, _functionName + strlen(_functionName));
		WString fnStr = fnStream.str();

		ss << _timeStamp << L" "
			<< typeStr << L" "
			<< threadNameStr << L"--- "
			<< fnStr << L" : "
			<< _message << L"\n";

		return ss.str();
	}
};
