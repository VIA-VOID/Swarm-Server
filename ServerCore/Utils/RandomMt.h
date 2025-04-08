#pragma once
#include <random>
#include "pch.h"

class RandomMT : public Singleton<RandomMT>
{
public:
	template <typename T>
	T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 engine(randomDevice());
		// T�� �������̸� uniform_int_distribution, 
		// �ƴ϶�� uniform_real_distribution�� ���
		using Distribution = typename std::conditional<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>::type;

		Distribution dist(min, max);
		return dist(engine);
	}
};
