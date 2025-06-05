#include "pch.h"
#include "Packet/PacketHandler.h"

class GameClient : public CoreService
{
public:
	GameClient()
		: CoreService(ServiceType::Client)
	{
		// 패킷 핸들러 초기화
		PacketHandler::Init();
		// JobGroup 초기화
		JobGroup::Init();
	}
	virtual ~GameClient() {}

	void OnConnected(SessionRef session) override
	{
		LOG_SYSTEM("Server Connect!! session: " + std::to_string(session->GetSessionID().GetID()));
	}

	void OnDisconnected(SessionRef session) override
	{
	}

	void OnRecv(SessionRef session, BYTE* buffer, int32 len) override
	{
	}

	void OnSend(SessionRef session, int32 len) override
	{
	}
};

int wmain()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));

	GameClient client;
	client.Run(7777, "127.0.0.1", 5);

	ThreadMgr.JoinAll();

	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//client.Stop();

	return 0;
}
