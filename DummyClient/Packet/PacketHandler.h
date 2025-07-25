#pragma once
#include "PacketId.h"

class PacketHandler;

using PacketFunc = std::function<void(SessionRef, BYTE*, uint16)>;
using PacketClass = std::vector<PacketHandlerURef>;

/*--------------------------------------------------------
					PacketHandler

- Protobuf 패킷, 패킷함수 관리
- 도메인별로 PacketHandler를 상속받아 핸들러 등록
- PacketHandler.cpp의 Init 구현부 자동생성
--------------------------------------------------------*/

class PacketHandler
{
public:
	virtual ~PacketHandler() = default;
	// 파생 클래스들의 테이블 등록, 초기화
	// 자동생성 코드
	static void Init();
	// 함수 테이블 등록
	// 상속받아 구현
	virtual void RegisterHandlers() = 0;
	// 함수 테이블에 등록된 함수 실행 (템플릿 HandlePacket 함수 실행)
	static void HandlePacket(SessionRef session, BYTE* buffer, uint16 len);
	// 패킷 전송
	template<typename T>
	static void SendPacket(SessionRef session, const T& packet, PacketID packetId);

protected:
	// 함수 테이블에 패킷 등록
	template<typename PacketType, typename HandleFunc>
	static void RegisterPacket(PacketID packetId, HandleFunc handle);
	// 전달받은 RunFunc 함수 실행
	template<typename PacketType, typename RunFunc>
	static void HandlePacket(RunFunc func, SessionRef session, BYTE* buffer, uint16 len);

protected:
	// 함수 테이블
	static PacketFunc _handlers[UINT16_MAX];
	// 도메인별 핸들러
	static PacketClass _domainHandlerClasses;
};

// 함수 테이블에 패킷 등록
template<typename PacketType, typename HandleFunc>
inline void PacketHandler::RegisterPacket(PacketID packetId, HandleFunc handle)
{
	_handlers[static_cast<uint16>(packetId)] = [handle](SessionRef session, BYTE* buffer, uint16 len)
		{
			HandlePacket<PacketType>(handle, session, buffer, len);
		};
}

// 전달받은 RunFunc 함수 실행
template<typename PacketType, typename RunFunc>
inline void PacketHandler::HandlePacket(RunFunc func, SessionRef session, BYTE* buffer, uint16 len)
{
	PacketType packet;
	BYTE* payload = buffer + sizeof(PacketHeader);
	uint16 payloadSize = len - sizeof(PacketHeader);
	std::string funcName(typeid(RunFunc).name());

	if (packet.ParseFromArray(payload, payloadSize) == false)
	{
		LOG_ERROR("Packet ParseFromArray 실패: " + funcName + ", packetSize: " + std::to_string(len));
	}

	func(session, packet);
}

// 패킷 전송
template<typename T>
inline void PacketHandler::SendPacket(SessionRef session, const T& packet, PacketID packetId)
{
	const uint16 payloadSize = static_cast<uint16>(packet.ByteSizeLong());
	const uint16 totalSize = sizeof(PacketHeader) + payloadSize;

	// 패킷 크기 검증
	if (totalSize > MAX_PACKET_SIZE)
	{
		LOG_ERROR("패킷 크기 초과: " + std::to_string(totalSize));
		return;
	}

	// sendBuffer 헤더 세팅
	SendBufferRef sendBuffer = ObjectPool<SendBuffer>::MakeShared(totalSize);
	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetWritePtr());
	header->size = totalSize;
	header->id = static_cast<uint16>(packetId);

	// 데이터 세팅
	BYTE* payload = sendBuffer->GetWritePtr() + sizeof(PacketHeader);
	packet.SerializeToArray(payload, payloadSize);
	sendBuffer->MoveWritePos(totalSize);

	// 데이터 전송
	session->Send(sendBuffer);
}
