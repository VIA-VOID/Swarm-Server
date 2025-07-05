#pragma once

/*--------------------------------------------------------
				GameServer

- 네트워크 콜백함수 구현부
--------------------------------------------------------*/
class GameServer : public CoreService
{
public:
	GameServer();
	virtual ~GameServer();

	void OnConnected(SessionRef session) override;
	void OnDisconnected(SessionRef session) override;
	void OnRecv(SessionRef session, BYTE* buffer, int32 len) override;
	void OnSend(SessionRef session, int32 len) override;

private:
	// 게임 종료시 주변 플레이어에게 알림
	void LeaveGame(const ObjectId objectId, const ZoneType zoneType, const Vector3d& position, const GridIndex& gridIndex);
};
