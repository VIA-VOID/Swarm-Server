#pragma once
#include "Log/LogDefine.h"
#include <chrono>

/*-------------------------------------
	싱글톤 패턴 적용된 클래스들 정의
-------------------------------------*/
#define DeadLockMgr						DeadlockDetector::GetInstance()
#define LogMgr							LogManager::GetInstance()
#define ThreadMgr						ThreadManager::GetInstance()
#define MemoryMgr						MemoryManager::GetInstance()
#define JobQ							JobQueue::GetInstance()

/*-------------------------------------
	함수 매크로
-------------------------------------*/
// 난수
#define RAND(min, max)					Utils::GetRandom<decltype(min)>(min, max)
// 현재시간
#define NOW								std::chrono::steady_clock::now()
// 로그
#define LOG_INFO(msg)					LogMgr.PushLog(LogType::Info, msg, __FUNCTION__)
#define LOG_SYSTEM(msg)					LogMgr.PushLog(LogType::System, msg, __FUNCTION__)
#define LOG_WARNING(msg)				LogMgr.PushLog(LogType::Warning, msg, __FUNCTION__)
#define LOG_ERROR(msg)					LogMgr.PushLog(LogType::Error, msg, __FUNCTION__)

/*-------------------------------------
	커스텀 함수 매크로
-------------------------------------*/
/// 인위적인 CRASH
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

/// 인위적인 조건부 CRASH
#define ASSERT_CRASH(expr)					\
{											\
	if(!(expr))								\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}

// JobQueue 타입-그룹 매핑
#define JOB_MAPPING(Type, Group)				\
    JobQ.RegisterTypeMapping<Type>(Group)		\


/*-------------------------------------
	조건부 커스텀 매크로
-------------------------------------*/
#if _DEBUG
// 데드락 탐지
#define USE_LOCK						Lock _mutex
#define LOCK_GUARD						LockGuard lockGuard(&_mutex, typeid(this).name())
#define UNIQUE_LOCK_GUARD				UniqueLockGuard ulockGuard(&_mutex, typeid(this).name())
#else
#define USE_LOCK						std::mutex _mutex
#define LOCK_GUARD						std::lock_guard<std::mutex> lockGuard(_mutex)
#define UNIQUE_LOCK_GUARD				std::unique_lock<std::mutex> ulockGuard(_mutex)
#endif
