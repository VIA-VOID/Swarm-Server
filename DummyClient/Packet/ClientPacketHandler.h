#pragma once
#include "Utils/Utils.h"

class Session;

using PacketFunc = std::function<void(Session*, BYTE*, uint16)>;

enum : uint16
{
	// 자동화 코드
	SC_CHAT_MSG = 0,
	SC_PLAYER_CREATE = 1,
	SC_PLAYER_MOVE = 2,
};

/*--------------------------------------------------------
					ClientPacketHandler

- protobuf 적용
- 컨텐츠 로직 함수
- 자동화 도구로 생성된 핸들러들이 여기 추가됨
	- SC: Server에서 Client로 패킷 전달
	- CS: Client에서 Server로 패킷 전달
--------------------------------------------------------*/

class ClientPacketHandler
{
public:
	// 함수 테이블 초기화
	static void Init()
	{
		// 자동화 코드
		_handlers[SC_CHAT_MSG] = [](Session* session, BYTE* buffer, int32 len)
			{
				ClientPacketHandler::HandlePacket<Protocol::SC_CHAT_MSG>(Handle_SC_CHAT_MSG, session, buffer, len);
			};
		_handlers[SC_PLAYER_CREATE] = [](Session* session, BYTE* buffer, int32 len)
			{
				ClientPacketHandler::HandlePacket<Protocol::SC_PLAYER_CREATE>(Handle_SC_PLAYER_CREATE, session, buffer, len);
			};
		_handlers[SC_PLAYER_MOVE] = [](Session* session, BYTE* buffer, int32 len)
			{
				ClientPacketHandler::HandlePacket<Protocol::SC_PLAYER_MOVE>(Handle_SC_PLAYER_MOVE, session, buffer, len);
			};
	}
	// 함수 테이블에 등록된 함수 실행 (템플릿 HandlePacket 함수 실행)
	static void HandlePacket(Session* session, BYTE* buffer, uint16 len);
	// 전달받은 RunFunc 함수 실행
	template<typename PacketType, typename RunFunc>
	static void HandlePacket(RunFunc func, Session* session, BYTE* buffer, uint16 len);
	// 패킷 전송
	template<typename T>
	static void SendPacket(Session* session, const T& packet, uint16 packetId);
	// 자동화 코드
	static void Handle_SC_CHAT_MSG(Session* session, Protocol::SC_CHAT_MSG& packet);
	static void Handle_SC_PLAYER_CREATE(Session* session, Protocol::SC_PLAYER_CREATE& packet);
	static void Handle_SC_PLAYER_MOVE(Session* session, Protocol::SC_PLAYER_MOVE& packet);

private:
	// 함수 테이블
	static PacketFunc _handlers[UINT16_MAX];
};

// 전달받은 RunFunc 함수 실행
template<typename PacketType, typename RunFunc>
inline void ClientPacketHandler::HandlePacket(RunFunc func, Session* session, BYTE* buffer, uint16 len)
{
	PacketType packet;
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	BYTE* payload = buffer + sizeof(PacketHeader);
	uint16 payloadSize = len - sizeof(PacketHeader);

	if (packet.ParseFromArray(payload, payloadSize) == false)
	{
		LOG_ERROR(L"Packet ParseFromArray 실패: " + Utils::ConvertUtf16(typeid(RunFunc).name()) + L", packetSize: " + Utils::ToWString(len));
	}

	func(session, packet);
}

// 패킷 전송
template<typename T>
inline void ClientPacketHandler::SendPacket(Session* session, const T& packet, uint16 packetId)
{
	const uint16 payloadSize = static_cast<uint16>(packet.ByteSizeLong());
	const uint16 totalSize = sizeof(PacketHeader) + payloadSize;

	// sendBuffer 헤더 세팅
	SendBufferRef sendBuffer = ObjectPool<SendBuffer>::MakeShared(totalSize + 1);
	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetWritePtr());
	header->size = totalSize;
	header->id = packetId;

	// 데이터 세팅
	BYTE* payload = sendBuffer->GetWritePtr() + sizeof(PacketHeader);
	packet.SerializeToArray(payload, payloadSize);

	// 데이터 전송
	sendBuffer->MoveWritePos(totalSize);
	session->Send(sendBuffer->GetReadPtr(), totalSize);
}
