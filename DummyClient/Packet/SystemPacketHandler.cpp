#include "pch.h"
#include "SystemPacketHandler.h"
#include "Utils/Timer.h"

// 하트비트 패킷
void SystemPacketHandler::Handle_SC_SYSTEM_PING(SessionRef session, const Protocol::SC_SYSTEM_PING& packet)
{
	Protocol::CS_SYSTEM_PONG pongPkt;
	pongPkt.set_currenttime(Timer::GetNowMsTime());

	PacketHandler::SendPacket(session, pongPkt, PacketID::CS_SYSTEM_PONG);
}
