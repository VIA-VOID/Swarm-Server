#include "pch.h"
#include "PacketHandler.h"

// 자동생성
// 도메인별 핸들러 include
#include "SystemPacketHandler.h"
#include "PlayerPacketHandler.h"
#include "ChatPacketHandler.h"

PacketFunc PacketHandler::_handlers[UINT16_MAX];
PacketClass PacketHandler::_domainHandlerClasses;

// 파생 클래스들의 테이블 등록, 초기화
// 자동생성 코드
void PacketHandler::Init()
{
	_domainHandlerClasses.emplace_back(ObjectPool<SystemPacketHandler>::MakeUnique());
	_domainHandlerClasses.emplace_back(ObjectPool<PlayerPacketHandler>::MakeUnique());
	_domainHandlerClasses.emplace_back(ObjectPool<ChatPacketHandler>::MakeUnique());

	// 도메인별로 함수 테이블 등록
	for (const auto& handler : _domainHandlerClasses)
	{
		handler->RegisterHandlers();
	}
}

// 함수 테이블에 등록된 함수 실행 (템플릿 HandlePacket 함수 실행)
void PacketHandler::HandlePacket(SessionRef session, BYTE* buffer, uint16 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	_handlers[header->id](session, buffer, len);
}
