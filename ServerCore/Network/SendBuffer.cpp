#include "pch.h"
#include "SendBuffer.h"

/*----------------------------
		SendBuffer
----------------------------*/

SendBuffer::SendBuffer(uint32 bufferSize/* = BUFFER_SIZE*/)
	: RingBuffer(bufferSize)
{
}

// 데이터 쓰기
bool SendBuffer::Write(const BYTE* src, uint32 size)
{
	LOCK_GUARD;
	return Enqueue(src, size);
}

// readPos 이동
void SendBuffer::CommitSend(uint32 size)
{
	LOCK_GUARD;
	MoveReadPos(size);
}

// cleanPos with Lock
void SendBuffer::CleanPosLock()
{
	LOCK_GUARD;
	CleanPos();
}

// 사용중인 용량 얻기 with Lock
uint32 SendBuffer::GetUseSizeLock()
{
	LOCK_GUARD;
	return GetUseSize();
}

BYTE* SendBuffer::GetSendPtr()
{
	LOCK_GUARD;
	return GetReadPtr();
}

uint32 SendBuffer::GetDirectSendSize()
{
	LOCK_GUARD;
	return GetDirectDequeSize();
}
