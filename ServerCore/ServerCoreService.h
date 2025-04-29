#pragma once

class Session;
class IocpServer;

/*-------------------------------------------------------
				NetworkService
--------------------------------------------------------*/

class NetworkService : public std::enable_shared_from_this<NetworkService>
{
public:
	NetworkService();
	~NetworkService() = default;
	virtual void StartServer(uint16 port) = 0;
	virtual void CloseServer() = 0;
};

/*-------------------------------------------------------
				ServerCoreService

- 컨텐츠 로직등에서 상속받아서 사용
--------------------------------------------------------*/

class ServerCoreService : NetworkService
{
public:
	ServerCoreService();
	virtual ~ServerCoreService() = default;
	void StartServer(uint16 port) override;
	void CloseServer() override;

	// 컨텐츠 로직에서 구현하여 사용
	virtual void OnConnected(Session* session) = 0;
	virtual void OnDisconnected(Session* session) = 0;
	virtual void OnRecv(Session* session, const BYTE* buffer, int32 len) = 0;
	virtual void OnSend(Session* session, int32 len) = 0;

private:
	IocpServer* _iocpServer;
};
