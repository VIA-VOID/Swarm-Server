#include "pch.h"
#include "SendBuffer.h"

/*----------------------------
		SendBuffer
----------------------------*/

SendBuffer::SendBuffer(uint32 bufferSize)
	: BaseBuffer(bufferSize)
{
}

// WSASend용 버퍼 정보 반환
void SendBuffer::GetWSABUF(WSABUF& wsabuf)
{
	wsabuf.buf = reinterpret_cast<CHAR*>(GetReadPtr());
	wsabuf.len = GetDataSize();
}

// WSASend 완료 후 읽기 위치 이동
void SendBuffer::OnSendCompleted(uint32 bytesTransferred)
{
	MoveReadPos(bytesTransferred);
}

// 송신 완료 여부
bool SendBuffer::IsCompleted() const
{
	return GetDataSize() == 0;
}
