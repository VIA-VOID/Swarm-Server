#pragma once

#include <chrono>

using SteadyClock = std::chrono::steady_clock;
using TimePoint = SteadyClock::time_point;

//------------------//
//		Clock		//
//------------------//
class Clock : public Singleton<Clock>
{
public:
	// �����õ� ����ð� ��ȯ (yyyy/mm/dd HH:MM:SS.MS)
	std::string GetFormattedTime() const;

	// �� �ð� ������ ����, �и��� ��ȯ
	int64 GetTimeDiff(const TimePoint& start, const TimePoint& end = NOW) const;
};

//------------------//
//		Timer		//
//------------------//
class Timer
{
public:
	Timer();
	// ���� �� ���ӽð� ����
	Timer(int64 durationMs);

	// Ÿ�̸� ����, �⺻�����ڷ� �����Ǿ����� ���
	void Start(int64 durationMs);

	// ���� Ȯ��
	bool IsExpired() const;

	// Ÿ�̸� ����
	void Reset();

	// ��� �ð�
	int64 Elapsed() const;

	// ���� �ð�
	int64 Remaining() const;

private:
	TimePoint _startTime;
	int64 _durationMs;
	bool _started;
};
