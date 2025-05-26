#include "pch.h"
#include "Job/ContentsJobGroups.h"
#include "Packet/PacketHandler.h"

class GameServer : public CoreService
{
public:
	GameServer()
		: CoreService(ServiceType::Server)
	{
		// 패킷 핸들러 초기화
		PacketHandler::Init();
		// JobGroup 초기화
		JobGroups::Init();
		// Job 스레드 생성 요청
		JobGroups::CreateThreadsForGroups();
	}
	virtual ~GameServer() {}
	
	void OnConnected(Session* session) override
	{
		LOG_SYSTEM(L"Client Connect!! session: " + Utils::ToWString(session->GetSessionID().GetID()));
	}

	void OnDisconnected(Session* session) override
	{
	}

	void OnRecv(Session* session, BYTE* buffer, int32 len) override
	{


		PacketHandler::HandlePacket(session, buffer, len);
	}

	void OnSend(Session* session, int32 len) override
	{
	}

};

int wmain()
{
	GameServer server;
	server.Run(7777);

	ThreadMgr.JoinAll();

	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//server.Stop();

	return 0;
}
