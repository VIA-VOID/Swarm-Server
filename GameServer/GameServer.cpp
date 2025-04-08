#include "pch.h"
#include "Utils/Timer.h"
#include "Utils/RandomMt.h"
#include <thread>

int main()
{
	int32 randomInt = RAND(1, 100);
	std::cout << "����: " << randomInt << std::endl;

	auto& clock = Clock::GetInstance();

	auto start = NOW;
	std::this_thread::sleep_for(std::chrono::milliseconds(123));
	auto end = NOW;

	std::cout << "���� �ð�: " << clock.GetFormattedTime() << std::endl;
	std::cout << "��� �ð�(ms): " << clock.GetTimeDiff(start, end) << std::endl;
	std::cout << "����� ���� �ð� ����(ms): " << clock.GetTimeDiff(start) << std::endl;

	const auto n = std::thread::hardware_concurrency();
	std::cout << n << " concurrent threads are supported.\n";

	Timer timer(3000); // 3�� Ÿ�̸�

	while (true)
	{
		if (timer.IsExpired())
		{
			std::cout << "Ÿ�̸� ����!" << std::endl;
			break;
		}

		std::cout << "���� �ð�: " << timer.Remaining() << " ms" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}
