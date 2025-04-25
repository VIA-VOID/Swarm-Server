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

void SendBuffer::CommitSend(uint32 size)
{
	LOCK_GUARD;
	MoveReadPos(size);
}

BYTE* SendBuffer::GetSendPtr()
{
	LOCK_GUARD;
	return GetReadPtr();
}

uint32 SendBuffer::GetDirectSendSize()
{
	LOCK_GUARD;
	return GetDirectEnqueSize();
}
