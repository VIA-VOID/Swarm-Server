#pragma once

class Session;
class IocpCore;

// dummyClient 생성시 필요
// 배치 크기, BATCH_SIZE 크기만큼 나누어 연결
constexpr uint32 BATCH_SIZE = 100;
// 배치 간 딜레이
constexpr uint32 BATCH_DELAY_MS = 100;

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

	void Run(uint16 port, const std::string& address = "", int16 connectCount = 1);
	void Stop();

	// 컨텐츠 로직에서 구현
	virtual void OnConnected(Session* session) = 0;
	virtual void OnDisconnected(Session* session) = 0;
	virtual void OnRecv(Session* session, const BYTE* buffer, int32 len) = 0;
	virtual void OnSend(Session* session, int32 len) = 0;

private:
	void DummyConnect(uint16 port, const std::string& address, int16 connectCount);

private:
	IocpCore* _iocpCore;
	ServiceType _type;
};
