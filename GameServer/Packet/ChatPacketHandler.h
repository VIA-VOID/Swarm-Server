#pragma once
#include "PacketHandler.h"

/*--------------------------------------------------------
				ChatPacketHandler

- 자동생성된 패킷 핸들러 클래스
- Handle_* 함수 구현
--------------------------------------------------------*/

class ChatPacketHandler : public PacketHandler
{
public:
	// 자동생성
	// 함수 테이블 등록
	void RegisterHandlers() override
	{
		RegisterPacket<Protocol::CS_CHAT_MSG>(PacketID::CS_CHAT_MSG, Handle_CS_CHAT_MSG);
	}

private:
	// 자동생성
	static void Handle_CS_CHAT_MSG(Session* session, Protocol::CS_CHAT_MSG& packet);
};
