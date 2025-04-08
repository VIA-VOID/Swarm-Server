#pragma once

template <typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

public:
	// ����, ���� ���� ����
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	static T& GetInstance()
	{
		static T instance;
		return instance;
	}
};
