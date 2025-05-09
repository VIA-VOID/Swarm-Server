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
		// MemoryManger에서 새로 할당
		return nullptr;
	}
	_useCount.fetch_add(1, std::memory_order_relaxed);
	_remainCount.fetch_sub(1, std::memory_order_relaxed);
	return _poolList.Pop();
}

// 메모리 반납
void MemoryPool::Push(void* ptr, bool counting /*= true*/)
{
	if (counting)
	{
		_useCount.fetch_sub(1, std::memory_order_relaxed);
	}
	_remainCount.fetch_add(1, std::memory_order_relaxed);
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
