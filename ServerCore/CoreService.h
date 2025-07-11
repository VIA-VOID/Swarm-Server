#pragma once

class Session;
class IocpCore;

enum ServiceType : uint16
{
	Client,
	Server
};

/*-------------------------------------------------------
				CoreService

- 컨텐츠 로직등에서 상속받아서 사용
--------------------------------------------------------*/

class CoreService : public std::enable_shared_from_this<CoreService>
{
public:
	CoreService(ServiceType type);
	virtual ~CoreService() = default;

	void Run(uint16 port, const std::string& address = "", uint16 connectCount = 1);
	void Stop();

	// 컨텐츠 로직에서 구현
	virtual void OnConnected(SessionRef session) = 0;
	virtual void OnDisconnected(SessionRef session) = 0;
	virtual void OnRecv(SessionRef session, BYTE* buffer, int32 len) = 0;
	virtual void OnSend(SessionRef session, int32 len) = 0;
	virtual void OnHeartbeat(SessionRef session) = 0;

	bool IsServerType() const;

private:
	void DummyConnect(uint16 port, const std::string& address, uint16 connectCount);

private:
	IocpCore* _iocpCore;
	ServiceType _type;
};
