#include "pch.h"
#include "SendBuffer.h"

/*----------------------------
		SendBuffer
----------------------------*/

SendBuffer::SendBuffer(uint32 bufferSize/* = BUFFER_SIZE*/)
	: RingBuffer(bufferSize), _dataSize(0)
{
}

// 데이터 쓰기
bool SendBuffer::Write(const BYTE* src, uint32 size)
{
	_dataSize = size;
	return Enqueue(src, size);
}

// readPos 이동
void SendBuffer::CommitSend(uint32 size)
{
	MoveReadPos(size);
}

// readPos 가져오기
BYTE* SendBuffer::GetSendPtr()
{
	return GetReadPtr();
}

// 전송하지 못한 데이터 크기
int32 SendBuffer::GetRemainSize()
{
	return _dataSize - _readPos;
}
