#pragma once
#include "Container/BaseBuffer.h"
#include <ws2def.h>

/*-------------------------------------------------------
				RecvBuffer
--------------------------------------------------------*/

class RecvBuffer : public BaseBuffer
{
public:
	RecvBuffer(uint32 bufferSize);
	virtual ~RecvBuffer() = default;

	// WSARecv용 버퍼 정보 반환
	void GetWSABUF(WSABUF& wsabuf);
	// WSARecv 완료 후 쓰기 위치 이동
	void OnRecvCompleted(uint32 bytesTransferred);
};
