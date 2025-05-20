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

	void Start(uint16 port, const std::string& address = "");
	void Stop();

	// 컨텐츠 로직에서 구현
	virtual void OnConnected(Session* session) = 0;
	virtual void OnDisconnected(Session* session) = 0;
	virtual void OnRecv(Session* session, const BYTE* buffer, int32 len) = 0;
	virtual void OnSend(Session* session, int32 len) = 0;

private:
	IocpCore* _iocpCore;
	ServiceType _type;
};
