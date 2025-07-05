#include "pch.h"
#include "GameClient.h"

int main()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));

	GameClient client;
	client.Run(7777, "127.0.0.1", 1500);

	ThreadMgr.JoinAll();

	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//client.Stop();

	return 0;
}
