#include "pch.h"
#include "Utils/CrashDump.h"
#include "Thread/Lock.h"
#include <cassert>
#include <condition_variable>

class CustomUniqueLockTester
{
private:
	USE_LOCK;

public:
	void RunAll()
	{
		TestConditionVariable();
		TestMultiThread();

		std::cout << "\n 모든 테스트 통과!" << std::endl;
	}

private:
	void TestConditionVariable()
	{
		ConditionVariable cv;
		bool ready = false;

		std::thread notifier([&]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			{
				UNIQUE_LOCK_GUARD;
				ready = true;
			}
			cv.notify_one();
			});

		UNIQUE_LOCK_GUARD;
		cv.wait(ulockGuard, [&]() { return ready; });

		assert(ready);
		std::cout << "[PASS] condition_variable_any wait 성공" << std::endl;
		notifier.join();
	}

	void TestMultiThread()
	{
		int sharedCounter = 0;
		const int threadCount = 10;
		const int iterationPerThread = 1000;
		std::vector<std::thread> threads;

		threads.reserve(threadCount);
		for (int i = 0; i < threadCount; ++i)
		{
			threads.emplace_back([&]() {
				for (int j = 0; j < iterationPerThread; ++j)
				{
					UNIQUE_LOCK_GUARD;
					++sharedCounter;
				}
				});
		}

		for (auto& t : threads)
			t.join();

		assert(sharedCounter == threadCount * iterationPerThread);
		std::cout << "[PASS] 멀티스레드 환경에서 락 보호 정상 작동" << std::endl;
	}
};

int main()
{
	CrashDump::Init();

	CustomUniqueLockTester tester;
	tester.RunAll();
	return 0;
}
