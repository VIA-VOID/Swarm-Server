#pragma once

// Protobuf 패킷
// - SC: Server에서 Client로 패킷 전달
// - CS : Client에서 Server로 패킷 전달
enum class PacketID : uint16
{
	SC_PLAYER_ENTER_GAME = 0,
	SC_PLAYER_SPAWN = 1,
	SC_PLAYER_DESPAWN = 2,
	SC_PLAYER_MOVE = 3,
	SC_CHAT_MSG = 4,
	CS_PLAYER_ENTER_GAME = 5,
	CS_PLAYER_MOVE = 6,
	CS_CHAT_MSG = 7,
};
