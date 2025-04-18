#pragma once
#include "Memory/MemoryLockStack.h"

/*-------------------------------------------------------
				MemoryPool

- MemoryManager에서 관리
- 미리 메모리를 할당하고 재사용 & 반납
--------------------------------------------------------*/

class MemoryPool
{
public:
	MemoryPool(uint32 allocSize)
		: _allocSize(allocSize), _useCount(0), _remainCount(0)
	{
	}
	// 메모리 대여
	void* Pop();
	// 메모리 반납
	void Push(void* ptr, bool counting = true);
	// 남아있는 풀 수량
	uint32 GetRemainCount();
	// 사용(대여) 수량
	uint32 GetUseCount();
	// 총 사이즈
	uint32 GetTotalCount();

private:
	// 사용(대여해준) 수량
	std::atomic<uint32> _useCount;
	// 남은 수량
	std::atomic<uint32> _remainCount;
	uint32 _allocSize;
	MemoryLockStack<void*> _poolList;
};
