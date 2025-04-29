#include "pch.h"

class ContentsServer : public ServerCoreService
{
public:
	ContentsServer()
	{
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
	ContentsServer server;
	server.StartServer(9999);


	std::this_thread::sleep_for(std::chrono::seconds(1));


	server.CloseServer();

	return 0;
}
