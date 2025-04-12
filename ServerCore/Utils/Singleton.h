#pragma once

template <typename T>
class Singleton
{
protected:
	Singleton() {}
	virtual ~Singleton() {}

public:
	// 복사, 복사 대입 방지
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	template <typename... Args>
	static T& GetInstance(Args&&... args)
	{
		static T instance(std::forward<Args>(args)...);
		return instance;
	}
};
