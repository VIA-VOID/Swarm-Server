#include "pch.h"
#include "MemoryHeader.h"

/*----------------------------
		MemoryHeader
----------------------------*/

// 실제 할당할 사이즈
uint32 MemoryHeader::GetRealSize(uint32 allocSize)
{
	uint32 realSize = sizeof(MemoryHeader) + allocSize;

#ifdef _DEBUG
	// 위, 아래로 메모리 guard
	realSize += GUARD_SIZE * 2;
#endif

	return realSize;
}

// header 붙이기
void* MemoryHeader::AttachHeader(void* ptr, uint32 blockSize, AllocType type)
{
	uint8* cursor = static_cast<uint8*>(ptr);

	// 헤더 세팅
	MemoryHeader* header = reinterpret_cast<MemoryHeader*>(cursor);
	header->allocSize = blockSize;
	header->allocType = type;

#ifdef _DEBUG
	uint8* nextHeaderPtr = reinterpret_cast<uint8*>(header + 1);
	// 위쪽 guard 메모리 초기화
	::memset(nextHeaderPtr, GUARD_PATTERN, GUARD_SIZE);
	// 아래쪽 guard 메모리 초기화
	uint8* dataPtr = nextHeaderPtr + GUARD_SIZE;
	uint8* overGuard = dataPtr + blockSize;
	::memset(overGuard, GUARD_PATTERN, GUARD_SIZE);

	return static_cast<void*>(dataPtr);
#else
	return static_cast<void*>(header + 1);
#endif 
}

// header 분리
MemoryHeader* MemoryHeader::DetachHeader(void* ptr)
{

	uint8* nextHeaderPtr = reinterpret_cast<uint8*>(ptr);

#ifdef _DEBUG
	nextHeaderPtr -= GUARD_SIZE;
#endif

	MemoryHeader* header = reinterpret_cast<MemoryHeader*>(nextHeaderPtr) - 1;
	uint32 size = header->allocSize;

#ifdef _DEBUG
	// 반납시 메모리 오염 확인
	uint8* under = reinterpret_cast<uint8*>(header + 1);
	uint8* over = reinterpret_cast<uint8*>(ptr) + size;

	for (int guard = 0; guard < GUARD_SIZE; guard++)
	{
		ASSERT_CRASH(under[guard] == GUARD_PATTERN);
		ASSERT_CRASH(over[guard] == GUARD_PATTERN);
	}
#endif

	return header;
}
