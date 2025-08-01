#pragma once
#include <random>

/*--------------------------------------------------------
						Utils

- 공통적으로 사용하는 함수 모음
--------------------------------------------------------*/
class Utils
{
public:
	// min, max 사이 랜덤값 리턴
	template <typename T>
	static T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 engine(randomDevice());
		// T가 정수형이면 uniform_int_distribution, 
		// 아니라면 uniform_real_distribution을 사용
		using Distribution = typename std::conditional<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>::type;

		Distribution dist(min, max);
		return dist(engine);
	}

	// 지정한 경로 리턴
	static std::string SetFilePath();

private:
	// 마지막 디렉토리 구분자 이전까지 자르기
	static std::string RemoveLastPathComponent(std::string& path);
	// 경로를 levels 상위로 이동
	static std::string GoUpDirectories(std::string& path, uint8 levels);
};
