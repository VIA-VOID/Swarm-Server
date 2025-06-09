#pragma once

/*--------------------------------------------------------
				GameServer

- 네트워크 콜백함수 구현부
--------------------------------------------------------*/
class GameServer : public CoreService
{
public:
	GameServer();
	virtual ~GameServer() = default;

	void OnConnected(SessionRef session) override;
	void OnDisconnected(SessionRef session) override;
	void OnRecv(SessionRef session, BYTE* buffer, int32 len) override;
	void OnSend(SessionRef session, int32 len) override;
};
