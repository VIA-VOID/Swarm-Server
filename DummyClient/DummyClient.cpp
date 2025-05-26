#include "pch.h"
#include "Job/ContentsJobGroups.h"
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
		JobGroups::Init();
		// Job 스레드 생성 요청
		JobGroups::CreateThreadsForGroups();
	}
	virtual ~GameClient() {}

	void OnConnected(Session* session) override
	{
		LOG_SYSTEM(L"Server Connect!! session: " + Utils::ToWString(session->GetSessionID().GetID()));
	}

	void OnDisconnected(Session* session) override
	{
	}

	void OnRecv(Session* session, BYTE* buffer, int32 len) override
	{
	}

	void OnSend(Session* session, int32 len) override
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
