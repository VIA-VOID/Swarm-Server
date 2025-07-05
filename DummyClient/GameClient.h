#pragma once

class GameClient : public CoreService
{
public:
	GameClient();
	virtual ~GameClient();

	void OnConnected(SessionRef session) override;
	void OnDisconnected(SessionRef session) override;
	void OnRecv(SessionRef session, BYTE* buffer, int32 len) override;
	void OnSend(SessionRef session, int32 len) override;
};