#pragma once

template <typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

public:
	// 복사, 복사 대입 방지
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	static T& GetInstance()
	{
		static T instance;
		return instance;
	}
};
