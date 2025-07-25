#pragma once

/*-------------------------------------------------------
				DummyChat

- Dummy 캐릭터 무작위 채팅
--------------------------------------------------------*/
class DummyChat : public std::enable_shared_from_this<DummyChat>
{
public:
	DummyChat(std::string playerName);
	
	// 무작위 채팅
	void DummyRandomChat(SessionRef session);

private:
	static const std::vector<std::string> _chatMessages;
	std::string _playerName;
	int32 _intervalTime;
};
