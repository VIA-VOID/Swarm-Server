#include "pch.h"
#include "ServerPacketHandler.h"

PacketFunc ServerPacketHandler::_handlers[UINT16_MAX];

void ServerPacketHandler::HandlePacket(Session* session, BYTE* buffer, uint16 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	_handlers[header->id](session, buffer, len);
}

void ServerPacketHandler::Handle_SC_PLAYER(Session* session, Protocol::SC_PLAYER& packet)
{
}
