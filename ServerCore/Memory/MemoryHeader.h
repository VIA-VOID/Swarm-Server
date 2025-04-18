#pragma once

/*-------------------------------------------------------
				MemoryHeader

- [MemoryHeader][DATA]
	- 4byte + dataSize

- debug모드에선 위아래로 guard가 붙음
- [MemoryHeader][GUARD][DATA][GUARD]
	- 4byte + 8byte + dataSize + 8byte

--------------------------------------------------------*/
class MemoryHeader
{
public:
	// 실제 할당할 사이즈
	static uint32 GetRealSize(uint32 allocSize);
	// header 붙이기
	static void* AttachHeader(void* ptr, uint32 blockSize);
	// header 분리
	static MemoryHeader* DetachHeader(void* ptr);

public:
	uint32 allocSize;
};
