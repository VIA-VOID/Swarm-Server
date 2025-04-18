#pragma once
#include <functional>

/*-------------------------------------------------------
				ThreadManager
--------------------------------------------------------*/
class ThreadManager : public Singleton<ThreadManager>
{
public:
	// 스레드 생성 & 일감 투척
	void Push(std::function<void(void)> callback);
	// 스레드 실행 완료 대기
	void Join();

private:
	USE_LOCK;
	Vector<std::thread> _threads;
};
