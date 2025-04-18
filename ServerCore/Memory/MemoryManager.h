#pragma once
#include "MemoryPool.h"

// BLOCK 단위를 나눈 개수
constexpr uint16 DIVIDED_NUM = 7;
#ifdef _DEBUG
constexpr uint8 GUARD_SIZE = 8;				// under,overflow 감지 size
constexpr uint8 GUARD_PATTERN = 0xDF;		// 메모리값, 오염 감지용
#endif

/*-------------------------------------------------------
				MemoryManager

- 서버시작시 1회 CHUNK_SIZE 만큼 한번에 할당
- 이후 BLOCK 크기만큼 쪼개어 pool로 관리
- MAX_BLOCK_SIZE 미만의 크기만 pool에 배치, 이상의 크기는 직접 할당(pool 관리 X)
- 모니터링 지표에 따라 chunk, block 조절
--------------------------------------------------------*/
class MemoryManager : public Singleton<MemoryManager>
{
	enum
	{
		PAGE_SIZE = 4096,				// 4KB, 페이지 크기
		CHUNK_SIZE = PAGE_SIZE * 16,	// 64KB, 전체 할당받을 크기
		MIN_BLOCK_SIZE = 16,			// BLOCK 최소 크기
		MAX_BLOCK_SIZE = 2048,			// BLOCK 최대 크기
	};

public:
	virtual ~MemoryManager();

	// 초기화
	// BLOCK 개수만큼 새로 할당
	void Init();
	// pool에서 메모리 꺼내기
	// 4K 이상은 새로 할당, pool 관리하지 않음
	void* Allocate(uint32 size);
	// pool에 메모리 반납
	void Release(void* ptr);

private:
	// CHUNK_SIZE 만큼 한번에 할당해서 pool에 저장
	void PushChunk();
	// size를 pool index번호로 변환
	uint16 SizeToIndex(uint32 size);

private:
	// 메모리 풀 테이블
	// 크기에 따라 관리
	MemoryPool* _poolTable[DIVIDED_NUM];
	void* _chunkPtr = nullptr;
};

/*-------------------------------------------------------
				MemoryHeader

- [MemoryHeader][DATA]
	- 4byte + dataSize

- debug모드에선 위아래로 guard가 붙음
- [MemoryHeader][GUARD][DATA][GUARD]
	- 4byte + 8byte + dataSize + 8byte

--------------------------------------------------------*/
struct MemoryHeader
{
	// 실제 할당할 사이즈
	static uint32 GetRealSize(uint32 allocSize)
	{
		uint32 realSize = sizeof(MemoryHeader) + allocSize;

#ifdef _DEBUG
		// 위, 아래로 메모리 guard
		realSize += GUARD_SIZE * 2;
#endif

		return realSize;
	}

	// header 붙이기
	static void* AttachHeader(void* ptr, uint32 blockSize)
	{
		uint8* cursor = static_cast<uint8*>(ptr);

		// 헤더 세팅
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(cursor);
		header->allocSize = blockSize;

#ifdef _DEBUG
		uint8* nextHeaderPtr = reinterpret_cast<uint8*>(header + 1);
		// 위쪽 guard 메모리 초기화
		::memset(nextHeaderPtr, GUARD_PATTERN, GUARD_SIZE);
		// 아래쪽 guard 메모리 초기화
		::memset(nextHeaderPtr + GUARD_SIZE + blockSize, GUARD_PATTERN, GUARD_SIZE);

		return static_cast<void*>(nextHeaderPtr + GUARD_SIZE);
#else
		return static_cast<void*>(header + 1);
#endif 
	}

	// header 분리
	static MemoryHeader* DetachHeader(void* ptr)
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
			ASSERT_CRASH(*under == *over);
		}
#endif

		return header;
	}

	uint32 allocSize;
};