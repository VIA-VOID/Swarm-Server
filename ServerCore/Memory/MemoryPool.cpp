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
	_useCount.fetch_add(1);
	_remainCount.fetch_sub(1);
	return _poolList.Pop();
}

// 메모리 반납
void MemoryPool::Push(void* ptr, bool counting /*= true*/)
{
	if (counting)
	{
		_useCount.fetch_sub(1);
	}
	_remainCount.fetch_add(1);
	_poolList.Push(ptr);
}

// 남아있는 풀 수량
uint32 MemoryPool::GetRemainCount()
{
	return _remainCount.load();
}

// 사용(대여) 수량
uint32 MemoryPool::GetUseCount()
{
	return _useCount.load();
}

// 총 사이즈
uint32 MemoryPool::GetTotalCount()
{
	return _remainCount.load() + _useCount.load();
}
