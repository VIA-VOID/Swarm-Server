#pragma once
#include "Pch/Types.h"
#include "Pch/CoreTLS.h"
#include "Container/Container.h"
#include <string>
#include <sstream>
#include <thread>
#include <iomanip>
#include <iostream>

enum class LogType : uint16
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
	const char* _functionName;

	LogMessage(LogType type, std::wstring msg, std::wstring timeStamp, ThreadId id, const char* fnName)
		: _type(type), _message(msg), _timeStamp(timeStamp), _functionName(fnName)
	{
	}

	std::wstring ToWString() const
	{
		std::wstringstream ss;

		// 로그 타입 문자열
		std::wstring typeStr = L"UNKNOWN";
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
		std::wstringstream tnStream;
		tnStream << std::left << std::setw(6) << std::wstring(LThreadName.begin(), LThreadName.end());
		std::wstring threadNameStr = tnStream.str();

		// function name 좌측 정렬 (30자 고정 너비)
		std::wstringstream fnStream;
		fnStream << std::left << std::setw(30) << std::wstring(_functionName, _functionName + strlen(_functionName));
		std::wstring fnStr = fnStream.str();

		ss << _timeStamp << L" "
			<< typeStr << L" "
			<< threadNameStr << L"--- "
			<< fnStr << L" : "
			<< _message << L"\n";

		return ss.str();
	}
};
