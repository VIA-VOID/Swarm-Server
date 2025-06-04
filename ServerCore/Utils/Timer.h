#pragma once

/*--------------------------------------------------------
					Clock
--------------------------------------------------------*/
class Clock
{
public:
	// 포맷팅된 현재시간 반환 (yyyy/mm/dd HH:MM:SS.MS)
	static std::string GetFormattedTime(char dateSep = '/', char timeSep = L':');
	// 포맷팅된 현재시간 반환 (yyyy/mm/dd)
	static std::string GetFormattedDate(char dateSep = '/');
	// 일자 변경 여부
	static bool IsNewDay();
	// 두 시간 사이의 차이, 밀리초 반환
	static int64 GetTimeDiff(const TimePoint& start, const TimePoint& end = NOW);
};

/*--------------------------------------------------------
					Timer
--------------------------------------------------------*/
class Timer
{
public:
	Timer();
	// 생성 시 지속시간 설정
	Timer(int64 durationMs);

	// 타이머 시작, 기본생성자로 생성되었을때 사용
	void Start(int64 durationMs);

	// 만료 확인
	bool IsExpired();

	// 타이머 리셋
	void Reset();

	// 경과 시간
	int64 Elapsed();

	// 남은 시간
	int64 Remaining();

private:
	TimePoint _startTime;
	int64 _durationMs;
	bool _started;
};
