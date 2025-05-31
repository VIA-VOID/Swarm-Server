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
		RegisterPacket<Protocol::SC_CHAT_MSG>(PacketID::SC_CHAT_MSG, Handle_SC_CHAT_MSG);
	}

private:
	// 자동생성
	static void Handle_SC_CHAT_MSG(Session* session, Protocol::SC_CHAT_MSG& packet);
};
