#include "pch.h"
#include "RecvBuffer.h"

/*----------------------------
		RecvBuffer
----------------------------*/

RecvBuffer::RecvBuffer(uint32 bufferSize)
	: BaseBuffer(bufferSize)
{
}

// WSARecv용 버퍼 정보 반환
void RecvBuffer::GetWSABUF(WSABUF& wsabuf)
{
	wsabuf.buf = reinterpret_cast<CHAR*>(GetWritePtr());
	wsabuf.len = GetFreeSize();
}

// WSARecv 완료 후 쓰기 위치 이동
void RecvBuffer::OnRecvCompleted(uint32 bytesTransferred)
{
	MoveWritePos(bytesTransferred);
}
