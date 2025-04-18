#pragma once

/*-------------------------------------------------------
				MemoryPool

- MemoryManager에서 관리
- 미리 메모리를 할당하고 재사용 & 반납
--------------------------------------------------------*/

class MemoryPool
{
public:
	MemoryPool(uint32 allocSize)
		: _allocSize(allocSize)
	{
	}
	// 메모리 대여
	void* Pop();
	// 메모리 반납
	void Push(void* ptr);

private:
	uint32 _allocSize;
	LockStack<void*> _poolList;
};
