#include "pch.h"
#include "MemoryManager.h"

void* PoolAllocator::Alloc(int32 size)
{
	return MEMORY_MANAGER.Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	return MEMORY_MANAGER.Release(ptr);
}
