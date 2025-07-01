#include "pch.h"
#include "GameServer.h"

int main()
{
	GameServer server;
	server.Run(7777);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	ThreadMgr.JoinAll();

	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//server.Stop();

	return 0;
}
