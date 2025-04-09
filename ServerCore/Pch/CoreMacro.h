#pragma once
#include <chrono>

/*-------------------------------------
	싱글톤 패턴 적용된 클래스들 정의
-------------------------------------*/
#define CLOCK				Clock::GetInstance()
#define RANDOM				RandomMT::GetInstance()
#define CRASHDUMP			CrashDump::GetInstance()

/*-------------------------------------
	함수 매크로
-------------------------------------*/
// 난수
#define RAND(min, max)		RANDOM.GetRandom<decltype(min)>(min, max)
// 현재시간
#define NOW					std::chrono::steady_clock::now()
