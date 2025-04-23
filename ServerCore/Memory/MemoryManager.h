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
	// 초기화
	// BLOCK 개수만큼 새로 할당
	void Init() override;
	// pool에서 메모리 꺼내기
	// 4K 이상은 새로 할당, pool 관리하지 않음
	void* Allocate(uint32 size);
	// pool에 메모리 반납
	void Release(void* ptr);
	// 종료
	void Shutdown() override;

private:
	// CHUNK_SIZE 만큼 한번에 할당해서 pool에 저장
	void PushChunk();
	// size를 pool index번호로 변환
	uint16 SizeToIndex(uint32 size);

private:
	// 메모리 풀 테이블
	// 크기에 따라 관리
	MemoryPool* _poolTable[DIVIDED_NUM] = {};
	void* _chunkPtr = nullptr;
};
