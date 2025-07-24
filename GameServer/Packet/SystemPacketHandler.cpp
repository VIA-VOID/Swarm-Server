#include "pch.h"
#include "SystemPacketHandler.h"
#include "Utils/Timer.h"

// 하트비트 패킷
void SystemPacketHandler::Handle_CS_SYSTEM_PONG(SessionRef session, const Protocol::CS_SYSTEM_PONG& packet)
{
	int64 clientTime = packet.currenttime();
	int64 serverTime = Timer::GetNowMsTime();
	int32 diffTime = static_cast<int32>(serverTime - clientTime);

	session->IncreasePongCount();
	session->updateRoundTripTime(std::abs(diffTime));
}
