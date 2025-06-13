#include "pch.h"
#include "RecvBuffer.h"
#include "Container/RingBuffer.h"

/*----------------------------
		RecvBuffer
----------------------------*/

RecvBuffer::RecvBuffer(uint32 bufferSize)
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

// 패킷 크기 만큼의 연속 공간이 있는지 확인
bool RecvBuffer::CanRecvPacketSize(uint32 packetSize)
{
	return GetUseSize() >= packetSize;
}
