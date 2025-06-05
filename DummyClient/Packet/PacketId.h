#pragma once

// Protobuf 패킷
// - SC: Server에서 Client로 패킷 전달
// - CS : Client에서 Server로 패킷 전달
enum class PacketID : uint16
{
	CS_CHAT_MSG = 0,
	CS_PLAYER_ENTER_GAME = 1,
	CS_PLAYER_MOVE = 2,
	SC_CHAT_MSG = 3,
	SC_PLAYER_ENTER_GAME = 4,
	SC_PLAYER_MOVE = 5,
};
