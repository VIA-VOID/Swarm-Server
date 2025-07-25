#include "pch.h"
#include "DummyChat.h"
#include "Utils/Utils.h"

const std::vector<std::string> DummyChat::_chatMessages = {
	u8"안녕하세요!",
	u8"반갑습니다~",
	u8"여기 사람 많네요",
	u8"같이 사냥하실분?",
	u8"아이템 교환 하실분?",
	u8"pk 하실분?",
	u8"보스 잡으러 가요",
	u8"도움 필요해요",
	u8"ㅎㅇㅎㅇ",
	u8"ㅋㅋㅋㅋㅋ",
	u8"ㅇㅇ",
	u8"ㄳㄳ",
};

DummyChat::DummyChat(std::string playerName)
	: _playerName(playerName)
{
	// 1 ~ 3초
	_intervalTime = Utils::GetRandom<int32>(1000, 5000);
}

// 무작위 채팅
void DummyChat::DummyRandomChat(SessionRef session)
{
	JobQ.DoAsyncAfter(_intervalTime, [self = shared_from_this(), session]() {
		int32 randomIndex = Utils::GetRandom<int32>(0, static_cast<int32>(_chatMessages.size() - 1));
		
		Protocol::CS_CHAT_MSG msgPkt;
		msgPkt.set_msg(_chatMessages[randomIndex]);
		msgPkt.set_msgtype(Protocol::MSG_TYPE_General);

		PacketHandler::SendPacket(session, msgPkt, PacketID::CS_CHAT_MSG);

		self->DummyRandomChat(session);
	}, JobGroup::Social);
}
