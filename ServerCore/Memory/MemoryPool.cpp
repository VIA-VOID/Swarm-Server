#include "pch.h"
#include "MemoryPool.h"

/*----------------------------
		MemoryPool
----------------------------*/

// 메모리 대여
void* MemoryPool::Pop()
{
	if (_poolList.Empty())
	{
		LOG_WARNING(L"메모리풀 비었음. 할당할 메모리 부족");
		// MemoryManger에서 새로 할당
		return nullptr;
	}
	return _poolList.Pop();
}

// 메모리 반납
void MemoryPool::Push(void* ptr)
{
	_poolList.Push(ptr);
}
