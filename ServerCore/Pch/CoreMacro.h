#pragma once
#include <chrono>

/*-------------------------------------
	�̱��� ���� ����� Ŭ������ ����
-------------------------------------*/
#define CLOCK				Clock::GetInstance()
#define RANDOM				RandomMT::GetInstance()


/*-------------------------------------
	�Լ� ��ũ��
-------------------------------------*/
// ����
#define RAND(min, max)		RANDOM.GetRandom<decltype(min)>(min, max)
// ����ð�
#define NOW					std::chrono::steady_clock::now()
