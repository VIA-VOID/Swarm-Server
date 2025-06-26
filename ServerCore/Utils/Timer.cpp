#include "pch.h"
#include "Timer.h"

#include <sstream>
#include <iomanip>

using namespace std::chrono;

/*----------------------------
		Clock
----------------------------*/

// 포맷팅된 현재시간 반환 (yyyy/mm/dd HH:MM:SS.MS)
std::string Clock::GetFormattedTime(char dateSep /*= '/'*/, char timeSep /*= L':'*/)
{
	system_clock::time_point now = system_clock::now();
	auto ms = ::duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	std::time_t now_c = system_clock::to_time_t(now);
	std::tm time_tm{};
	::localtime_s(&time_tm, &now_c);

	char buffer[32];
	std::snprintf(buffer, sizeof(buffer),
		"%04d%c%02d%c%02d %02d%c%02d%c%02d.%03d",
		time_tm.tm_year + 1900, dateSep,
		time_tm.tm_mon + 1, dateSep,
		time_tm.tm_mday,
		time_tm.tm_hour, timeSep,
		time_tm.tm_min, timeSep,
		time_tm.tm_sec,
		static_cast<int>(ms.count())
	);

	return std::string(buffer);
}

// 포맷팅된 현재시간 반환 (yyyy/mm/dd)
std::string Clock::GetFormattedDate(char dateSep /*= '/'*/)
{
	system_clock::time_point now = system_clock::now();

	std::time_t now_c = system_clock::to_time_t(now);
	std::tm time_tm{};
	::localtime_s(&time_tm, &now_c);

	char buffer[16];
	std::snprintf(buffer, sizeof(buffer),
		"%04d%c%02d%c%02d",
		time_tm.tm_year + 1900, dateSep,
		time_tm.tm_mon + 1, dateSep,
		time_tm.tm_mday
	);

	return std::string(buffer);
}

// 일자 변경 여부
bool Clock::IsNewDay()
{
	LToday = GetFormattedDate();
	std::string now = GetFormattedDate();

	// 일자변경
	if (LToday != now)
	{
		LToday = now;
		return true;
	}
	return false;
}

// 두 시간 사이의 차이, 밀리초 반환
int64 Clock::GetTimeDiff(const TimePoint& start, const TimePoint& end /*= NOW*/)
{
	return ::duration_cast<milliseconds>(end - start).count();
}

/*----------------------------
		Timer
----------------------------*/

Timer::Timer()
	: _durationMs(0), _started(false)
{
}

// 생성 시 지속시간 설정
Timer::Timer(int64 durationMs)
	: _startTime(NOW), _durationMs(durationMs), _started(true)
{
}

// 타이머 시작, 기본생성자로 생성되었을때 사용
void Timer::Start(int64 durationMs)
{
	_startTime = NOW;
	_durationMs = durationMs;
	_started = true;
}

// 만료 확인
bool Timer::IsExpired()
{
	if (_started == false)
	{
		return false;
	}

	int64 elapsed = Clock::GetTimeDiff(_startTime);
	return elapsed >= _durationMs;
}

// 타이머 리셋
void Timer::Reset()
{
	if (_started)
	{
		_startTime = NOW;
	}
}

// 경과 시간
int64 Timer::Elapsed()
{
	if (_started == false)
	{
		return 0;
	}

	return Clock::GetTimeDiff(_startTime);
}

// 남은 시간
int64 Timer::Remaining()
{
	if (_started == false)
	{
		return 0;
	}

	int64 remaining = _durationMs - Elapsed();
	return (remaining > 0) ? remaining : 0;
}
