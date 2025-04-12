#pragma once
#include <chrono>

/*-------------------------------------
	싱글톤 패턴 적용된 클래스들 정의
-------------------------------------*/
#define CLOCK					Clock::GetInstance()
#define RANDOM					RandomMT::GetInstance()
#define LOCK					LockDebugManger::GetInstance()

/*-------------------------------------
	함수 매크로
-------------------------------------*/
// 난수
#define RAND(min, max)			RANDOM.GetRandom<decltype(min)>(min, max)
// 현재시간
#define NOW						std::chrono::steady_clock::now()

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

/*-------------------------------------
	커스텀 매크로
-------------------------------------*/
// debug 모드시 데드락 탐지
#define USE_LOCK				Lock _lock
#define LOCK_GURAD				LockGuard lockGuard(&_lock)
