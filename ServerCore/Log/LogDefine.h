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
	std::string _functionName;

	LogMessage(LogType type, std::string msg, std::string timeStamp, std::string fnName)
		: _type(type), _message(msg), _timeStamp(timeStamp), _functionName(fnName)
	{
	}

	std::string ToString() const
	{
		std::string typeStr = "UNKNOWN";
		switch (_type)
		{
		case LogType::Info:
			typeStr = "INFO";
			break;
		case LogType::System:
			typeStr = "SYSTEM";
			break;
		case LogType::Warning:
			typeStr = "WARNING";
			break;
		case LogType::Error:
			typeStr = "ERROR";
			break;
		}
		// 로그타입 패딩
		typeStr.resize(8, ' ');

		// 스레드명
		std::string threadName(LThreadName.begin(), LThreadName.end());
		if (threadName.length() > 12) 
		{
			threadName = threadName.substr(0, 12);
		}
		// 스레드명 패딩
		if (threadName.length() < 12)
		{
			threadName.append(12 - threadName.length(), ' ');
		}

		// 함수명
		std::string fnName(_functionName);
		if (fnName.length() > 40) 
		{
			fnName = fnName.substr(0, 40);
		}
		// 함수명 패딩
		if (fnName.length() < 40)
		{
			fnName.append(40 - fnName.length(), ' ');
		}

		std::string result;
		result.reserve(256);
		result = _timeStamp + " " + typeStr + " " + threadName + "--- " + fnName + " : " + _message + "\n";
		return result;
	}
};
