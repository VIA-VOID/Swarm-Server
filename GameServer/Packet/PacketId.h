#pragma once

// Protobuf 패킷
// - SC: Server에서 Client로 패킷 전달
// - CS : Client에서 Server로 패킷 전달
enum class PacketID : uint16
{
	SC_SYSTEM_PING = 0,
	SC_PLAYER_ENTER_GAME = 1,
	SC_OBJECT_SPAWN = 2,
	SC_OBJECT_DESPAWN = 3,
	SC_PLAYER_MOVE = 4,
	SC_CHAT_MSG = 5,
	CS_SYSTEM_PONG = 6,
	CS_PLAYER_ENTER_GAME = 7,
	CS_PLAYER_MOVE = 8,
	CS_CHAT_MSG = 9,
};
