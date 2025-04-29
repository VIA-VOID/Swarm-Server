#pragma once
#include "pch/Types.h"
class SendBuffer;
class Session;

// CPU 스레드 개수
const uint16 CPU_THREAD_COUNT = std::thread::hardware_concurrency();
// 버퍼 크기 (8KB)
constexpr uint32 BUFFER_SIZE = 8196;
// 타임아웃 시간 (300초)
constexpr std::chrono::seconds TIMEOUT_SECONDS(300);
// 최대 패킷 크기 (2KB)
constexpr uint32 MAX_PACKET_SIZE = 2048;
// Accept 버퍼 크기
constexpr uint32 ACCEPT_BUFFER_SIZE = 128;
// AcceptThread 개수
constexpr uint32 ACCEPT_THREAD_NUM = 1;
// WSABUF로 한번에 보낼 조각 개수
// Scatter-Gather
constexpr uint32 MAX_SEND_BUFFER_COUNT = 10;
// GQCS WorkerThread 개수
const uint16 MAX_WORKER_THREAD_NUM = CPU_THREAD_COUNT;
// 네트워크 I/O 타입
enum class NetworkIOType : uint16
{
	Accept = 0,
	Recv,
	Send
};
// 오버랩 확장 구조체
struct OverlappedEx
{
	OVERLAPPED overlapped = {};
	NetworkIOType type = NetworkIOType::Accept;
};
// 비동기 Send 구조체
struct SendContext
{
	OverlappedEx overlappedEx;
	Vector<SendBuffer*> buffers;
};
// 비동기 Accept 구조체
struct AcceptContext
{
	OverlappedEx overlappedEx;
	Session* session = nullptr;
	Array<BYTE, ACCEPT_BUFFER_SIZE> acceptBuffer = {};
};
// 세션 ID 클래스
class SessionID
{
public:
	SessionID() : _id(0) {}
	SessionID(uint64 id) : _id(id) {}
	uint64 GetID() const
	{
		return _id;
	}
	static SessionID Generate()
	{
		static std::atomic<uint64> atomicID = 1;
		return SessionID(atomicID.fetch_add(1, std::memory_order_relaxed));
	}
	bool operator==(const SessionID& other) const { return _id == other._id; }
	bool operator!=(const SessionID& other) const { return _id != other._id; }

private:
	uint64 _id;
};

namespace std {
	template<>
	struct hash<SessionID>
	{
		size_t operator()(const SessionID& id) const noexcept
		{
			return hash<uint64>()(id.GetID());
		}
	};
}
