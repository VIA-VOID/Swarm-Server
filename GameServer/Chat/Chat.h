#pragma once

/*--------------------------------------------------------
					Chat

- 채팅 관련 유틸리티
--------------------------------------------------------*/
class Chat
{
public:
	// 메시지 본문 만들기
	static std::string MakeMessage(const std::string& msg, const std::string& playerName, const Protocol::MsgType msgType);

private:
	// MsgType에 따라 접두사 만들기
	static std::string MakePrefix(const Protocol::MsgType msgType);

};
