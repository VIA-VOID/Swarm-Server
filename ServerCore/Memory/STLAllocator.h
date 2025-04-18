#pragma once

class PoolAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/*-------------------------------------------------------
				STLAllocator

- STL 컨테이너에 메모리 풀 적용
--------------------------------------------------------*/
template <typename T>
class STLAllocator
{
public:
	using value_type = T;

	STLAllocator() {}

	template <typename Other>
	STLAllocator(const STLAllocator<Other>&) {}

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}
};
