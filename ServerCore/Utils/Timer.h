#pragma once

/*--------------------------------------------------------
					Timer
--------------------------------------------------------*/
class Timer
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
	// 현재시간 밀리초 반환
	static int64 GetNowMsTime();
};
