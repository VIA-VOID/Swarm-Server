#include "pch.h"
#include "Chat.h"
#include "Utils/Timer.h"

// 메시지 본문 만들기
std::string Chat::MakeMessage(const std::string& msg, const std::string& playerName, const Protocol::MsgType msgType)
{
	std::string prefix = MakePrefix(msgType);
	std::string currentTime = Timer::GetFormattedHour();

	std::string resultMsg;
	size_t msgSize = prefix.size() + currentTime.size() + playerName.size() + msg.size() + 10;

	resultMsg.reserve(msgSize);
	resultMsg.append(u8"[");
	resultMsg.append(currentTime);
	resultMsg.append(u8"]");
	resultMsg.append(prefix);
	resultMsg.append(playerName);
	resultMsg.append(u8": ");
	resultMsg.append(msg);

	return resultMsg;
}

// MsgType에 따라 접두사 만들기
std::string Chat::MakePrefix(const Protocol::MsgType msgType)
{
	switch (msgType)
	{
	case Protocol::MSG_TYPE_General:
		return u8"[일반]";
	case Protocol::MSG_TYPE_Local:
		return u8"[지역]";
	case Protocol::MSG_TYPE_System:
		return u8"[시스템]";
	default:
		return u8"[모두]";
	}
}
