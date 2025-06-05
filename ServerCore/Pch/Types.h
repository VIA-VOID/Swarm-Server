#pragma once
#include <functional>
#include <chrono>

//----------------------------------------------------------//
// 자료형 정의
using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

//----------------------------------------------------------//
// 조건부 타입 정의
#ifdef _DEBUG
using ConditionVariable = std::condition_variable_any;
#else
using ConditionVariable = std::condition_variable;
#endif

//----------------------------------------------------------//
// 타입 정의
using ThreadId = std::thread::id;
using CallbackType = std::function<void()>;
using SteadyClock = std::chrono::steady_clock;
using TimePoint = SteadyClock::time_point;
using JobGroupId = uint16;

//----------------------------------------------------------//
// 스마트포인터
#define SHARED_PTR(name)		using name##Ref = std::shared_ptr<class name>;
#define WEAK_PTR(name)			using name##WRef = std::weak_ptr<class name>;
#define UNIQUE_PTR(name)		using name##URef = std::shared_ptr<class name>;

WEAK_PTR(Session)

SHARED_PTR(SendBuffer)
SHARED_PTR(Session)

UNIQUE_PTR(PacketHandler)