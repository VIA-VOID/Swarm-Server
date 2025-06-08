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
	std::string _message;
	std::string _timeStamp;
	const char* _functionName;

	LogMessage(LogType type, std::string msg, std::string timeStamp, ThreadId id, const char* fnName)
		: _type(type), _message(msg), _timeStamp(timeStamp), _functionName(fnName)
	{
	}

	std::string ToString() const
	{
		std::stringstream ss;

		// 로그 타입 문자열
		std::string typeStr = "UNKNOWN";
		switch (_type)
		{
		case LogType::Info:
			typeStr = " INFO    ";
			break;
		case LogType::System:
			typeStr = " SYSTEM  ";
			break;
		case LogType::Warning:
			typeStr = " WARNING ";
			break;
		case LogType::Error:
			typeStr = " ERROR   ";
			break;
		}

		// threadName 좌측 정렬 (12자 고정 너비)
		std::stringstream tnStream;
		tnStream << std::left << std::setw(12) << std::string(LThreadName.begin(), LThreadName.end());
		std::string threadNameStr = tnStream.str();

		// function name 좌측 정렬 (40자 고정 너비)
		std::stringstream fnStream;
		fnStream << std::left << std::setw(40) << std::string(_functionName, _functionName + strlen(_functionName));
		std::string fnStr = fnStream.str();

		ss << _timeStamp << " "
			<< typeStr << " "
			<< threadNameStr << "--- "
			<< fnStr << " : "
			<< _message << "\n";

		return ss.str();
	}
};
