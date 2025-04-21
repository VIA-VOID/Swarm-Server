#include "pch.h"
#include "MemoryManager.h"

void* PoolAllocator::Alloc(int32 size)
{
	return MemoryMgr.Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	return MemoryMgr.Release(ptr);
}
