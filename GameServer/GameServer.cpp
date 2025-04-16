#include "pch.h"

class ComplexWorker
{
public:
	ComplexWorker(int id) : _id(id) {}

	void A()
	{
		LOCK_GUARD;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << "A  \n";
		LOG_INFO(L"[A] THREAD");
		LOG_SYSTEM(L"[A] THREAD");
	}

	void B()
	{
		LOCK_GUARD;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << "B  \n";
		LOG_SYSTEM(L"[A] THREAD");
		LOG_INFO(L"[B] THREAD");
	}

	void C()
	{
		LOCK_GUARD;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << "C  \n";
		LOG_ERROR(L"[C] THREAD");
	}

	void D()
	{
		LOCK_GUARD;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << "D  \n";
		LOG_SYSTEM(L"[D] THREAD");
		LOG_INFO(L"[D] THREAD");
	}

	void RunLoop()
	{
		while (true)
		{
			A();
			B();
			C();
			D();
			std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 약간 쉬어감
		}
	}

private:
	USE_LOCK;
	int _id;
};

int wmain()
{
	std::wcout.imbue(std::locale("kor"));

	CrashDump::Init();

	LOG.Init();

	constexpr int ThreadCount = 8;

	std::vector<std::unique_ptr<ComplexWorker>> workers;

	for (int i = 0; i < ThreadCount; ++i)
	{
		workers.emplace_back(std::make_unique<ComplexWorker>(i));

		THREAD_MANAGER.Push([&, i]() {
			workers[i]->RunLoop();
			});
	}

	THREAD_MANAGER.Push([] {
		LOG.ProcessThread();
		});




	THREAD_MANAGER.Join();
	LOG.Shutdown();

	return 0;
}
