#include "pch.h"
#include "Job/ContentsJobGroups.h"

class ContentsServer : public ServerCoreService
{
public:
	ContentsServer()
	{
		// JobGroup 초기화
		JobGroups::Init();
		// Job 스레드 생성 요청
		JobGroups::CreateThreadsForGroups();
	}
	virtual ~ContentsServer() {}
	void OnConnected(Session* session) override
	{
	}

	void OnDisconnected(Session* session) override
	{
	}

	void OnRecv(Session* session, const BYTE* buffer, int32 len) override
	{
	}

	void OnSend(Session* session, int32 len) override
	{
	}
};

int wmain()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ContentsServer server;
	server.StartServer(9999);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	server.CloseServer();

	return 0;
}
