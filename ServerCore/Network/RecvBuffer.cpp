#include "pch.h"
#include "RecvBuffer.h"
#include "Container/RingBuffer.h"

/*----------------------------
		RecvBuffer
----------------------------*/

RecvBuffer::RecvBuffer(uint32 bufferSize/* = BUFFER_SIZE*/)
	: RingBuffer(bufferSize)
{
}

// 데이터 읽기
bool RecvBuffer::Read(BYTE* dest, uint32 destSize, uint32 size)
{
	if (size > GetUseSize())
	{
		return false;
	}
	// 데이터 빼오기
	Peek(dest, destSize, size);
	// readPos 이동
	MoveReadPos(size);
	return true;
}

// writePos 이동
void RecvBuffer::CommitRecv(uint32 size)
{
	MoveWritePos(size);
}

// 패킷 크기 만큼의 연속 공간이 있는지 확인
bool RecvBuffer::CanRecvPacketSize(uint32 packetSize)
{
	return GetUseSize() >= packetSize;
}

BYTE* RecvBuffer::GetRecvPtr()
{
	return GetWritePtr();
}

uint32 RecvBuffer::GetDirectRecvSize()
{
	return GetDirectEnqueSize();
}
