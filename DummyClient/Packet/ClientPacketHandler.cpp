#include "pch.h"
#include "ClientPacketHandler.h"

PacketFunc ClientPacketHandler::_handlers[UINT16_MAX];

void ClientPacketHandler::HandlePacket(Session* session, BYTE* buffer, uint16 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	_handlers[header->id](session, buffer, len);
}

void ClientPacketHandler::Handle_SC_CHAT_MSG(Session* session, Protocol::SC_CHAT_MSG& packet)
{
}

void ClientPacketHandler::Handle_SC_PLAYER_CREATE(Session* session, Protocol::SC_PLAYER_CREATE& packet)
{
}

void ClientPacketHandler::Handle_SC_PLAYER_MOVE(Session* session, Protocol::SC_PLAYER_MOVE& packet)
{
}
