#include "pch.h"
#include "MemoryManager.h"
#include "MemoryHeader.h"

// 블록단위 배열
constexpr uint16 _blockArray[DIVIDED_NUM] = { 32, 64, 128, 256, 512, 1024, 2048 };
// 비율단위 배열
// - CHUNK_SIZE를 블록단위당 몇 %로 가져갈지 작성
constexpr uint16 _ratioArray[DIVIDED_NUM] = { 8, 10, 14, 14, 18, 12, 8 };

/*----------------------------
		MemoryManager
----------------------------*/

// 초기화
// BLOCK 개수만큼 새로 할당
void MemoryManager::Init()
{
	for (uint16 i = 0; i < DIVIDED_NUM; i++)
	{
		_poolTable[i] = new MemoryPool(_blockArray[i]);
	}

	// CHUNK_SIZE 만큼 한번에 할당해서 pool에 저장
	PushChunk();
}

// pool에서 메모리 꺼내기
void* MemoryManager::Allocate(uint32 size)
{
	// MAX_BLOCK_SIZE 크기보다 클 경우 pool로 관리하지 않고 새로 할당
	if (size > MAX_BLOCK_SIZE)
	{
		uint32 realSize = MemoryHeader::GetRealSize(size);
		void* virtualPtr = ::VirtualAlloc(nullptr, realSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		// 헤더 붙이기
		return MemoryHeader::AttachHeader(virtualPtr, size, AllocType::Virtual);
	}
	// pool index
	uint16 idx = SizeToIndex(size);
	void* ptr = _poolTable[idx]->Pop();

	// 풀에 할당된 메모리 없음
	if (ptr == nullptr)
	{
		// 새로 할당해준다.
		uint32 realSize = MemoryHeader::GetRealSize(size);
		void* mallocPtr = ::malloc(realSize);

		// 헤더 붙이기
		return MemoryHeader::AttachHeader(mallocPtr, size, AllocType::Malloc);
	}

	return ptr;
}

// pool에 메모리 반납
void MemoryManager::Release(void* ptr)
{
	ASSERT_CRASH(ptr != nullptr);

	// 헤더 분리
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
	uint32 size = header->allocSize;
	AllocType type = header->allocType;

	// MAX_BLOCK_SIZE 크기보다 클 경우 pool로 관리하지 않고 반납
	if (size > MAX_BLOCK_SIZE)
	{
		::VirtualFree(ptr, 0, MEM_RELEASE);
		return;
	}

	// 할당 타입에 따라 해제
	switch (type)
	{
	case AllocType::Pool:
	{
		// Pool로 반납
		uint16 idx = SizeToIndex(size);
		_poolTable[idx]->Push(ptr);
	}
	break;

	case AllocType::Malloc:
	{
		void* originalPtr = static_cast<void*>(header);
		::free(originalPtr);
	}
	break;

	case AllocType::Virtual:
	{
		void* originalPtr = static_cast<void*>(header);
		::VirtualFree(originalPtr, 0, MEM_RELEASE);
	}
	break;
	}
}

// 종료
void MemoryManager::Shutdown()
{
}

// CHUNK_SIZE 만큼 한번에 할당해서 pool에 저장
void MemoryManager::PushChunk()
{
	// chunk 크기만큼 할당
	void* chunk = ::VirtualAlloc(nullptr, CHUNK_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	ASSERT_CRASH(chunk != nullptr);

	// 전체 비율
	uint16 totalWeight = 0;
	for (auto ratio : _ratioArray)
	{
		totalWeight += ratio;
	}

	// 메모리 위치 커서
	uint8* cursor = static_cast<uint8*>(chunk);
	uint32 chunkSize = CHUNK_SIZE;

	// 크기와 비율에 맞게 풀에 삽입
	for (uint16 div = 0; div < DIVIDED_NUM; div++)
	{
		uint32 blockSize = _blockArray[div];
		uint32 realBlockSize = MemoryHeader::GetRealSize(blockSize);
		uint16 ratio = _ratioArray[div];
		// 비율에 따라 할당할 사이즈
		double totalBytes = (static_cast<double>(CHUNK_SIZE) * ratio) / totalWeight;
		uint32 allocBytes = static_cast<uint32>(totalBytes);
		// 실제 할당 가능한 블록 개수 계산
		uint32 blockCount = allocBytes / realBlockSize;
		// 실제 할당될 메모리 크기
		uint32 realAllocBytes = blockCount * realBlockSize;
		// 메모리 부족시 break
		if (realAllocBytes > chunkSize)
		{
			break;
		}
		// 결정된 크기로 비율(개수)만큼 pool에 저장
		for (uint16 cnt = 0; cnt < blockCount; cnt++)
		{
			// 헤더 붙이기
			void* dataPtr = MemoryHeader::AttachHeader(cursor, blockSize, AllocType::Pool);
			// 풀에 저장
			_poolTable[div]->Push(dataPtr, false);
			// 포인터 이동
			cursor += realBlockSize;
		}
		// 할당된 메모리 감소
		chunkSize -= realAllocBytes;
	}
}

// size를 pool index번호로 변환
uint16 MemoryManager::SizeToIndex(uint32 size)
{
	for (uint16 index = 0; index < DIVIDED_NUM; index++)
	{
		if (size <= _blockArray[index])
		{
			return index;
		}
	}
	LOG_ERROR("블록 배열 매핑 실패!!");
	CRASH("NOT FOUND INDEX");
	return 0;
}
