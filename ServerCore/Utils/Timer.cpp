#include "pch.h"
#include "Timer.h"

#include <sstream>
#include <iomanip>

using namespace std::chrono;

//------------------//
//		Clock		//
//------------------//

// �����õ� ����ð� ��ȯ (yyyy/mm/dd HH:MM:SS.MS)
std::string Clock::GetFormattedTime() const
{
	system_clock::time_point now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	std::time_t now_c = system_clock::to_time_t(now);
	std::tm time_tm{};
	localtime_s(&time_tm, &now_c);

	std::ostringstream oss;
	oss << std::put_time(&time_tm, "%Y/%m/%d %H:%M:%S.");
	oss << std::setw(3) << std::setfill('0') << ms.count();

	return oss.str();
}

// �� �ð� ������ ����, �и��� ��ȯ
int64 Clock::GetTimeDiff(const TimePoint& start, const TimePoint& end /*= NOW*/) const
{
	return duration_cast<milliseconds>(end - start).count();
}

//------------------//
//		Timer		//
//------------------//

Timer::Timer()
	: _durationMs(0), _started(false)
{
}

// ���� �� ���ӽð� ����
Timer::Timer(int64 durationMs)
	: _startTime(NOW), _durationMs(durationMs), _started(true)
{
}

// Ÿ�̸� ����, �⺻�����ڷ� �����Ǿ����� ���
void Timer::Start(int64 durationMs)
{
	_startTime = NOW;
	_durationMs = durationMs;
	_started = true;
}

// ���� Ȯ��
bool Timer::IsExpired() const
{
	if (_started == false)
	{
		return false;
	}

	int64 elapsed = CLOCK.GetTimeDiff(_startTime);
	return elapsed >= _durationMs;
}

// Ÿ�̸� ����
void Timer::Reset()
{
	if (_started)
	{
		_startTime = NOW;
	}
}

// ��� �ð�
int64 Timer::Elapsed() const
{
	if (_started == false)
	{
		return 0;
	}

	return CLOCK.GetTimeDiff(_startTime);
}

// ���� �ð�
int64 Timer::Remaining() const
{
	if (_started == false)
	{
		return 0;
	}

	int64 remaining = _durationMs - Elapsed();
	return (remaining > 0) ? remaining : 0;
}
