#include "pch.h"
#include "Utils/Timer.h"
#include "Utils/RandomMt.h"
#include <thread>

int main()
{
	int32 randomInt = RAND(1, 100);
	std::cout << "난수: " << randomInt << std::endl;

	auto& clock = Clock::GetInstance();

	auto start = NOW;
	std::this_thread::sleep_for(std::chrono::milliseconds(123));
	auto end = NOW;

	std::cout << "현재 시간: " << clock.GetFormattedTime() << std::endl;
	std::cout << "경과 시간(ms): " << clock.GetTimeDiff(start, end) << std::endl;
	std::cout << "현재와 시작 시간 차이(ms): " << clock.GetTimeDiff(start) << std::endl;

	const auto n = std::thread::hardware_concurrency();
	std::cout << n << " concurrent threads are supported.\n";

	Timer timer(3000); // 3초 타이머

	while (true)
	{
		if (timer.IsExpired())
		{
			std::cout << "타이머 만료!" << std::endl;
			break;
		}

		std::cout << "남은 시간: " << timer.Remaining() << " ms" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}
