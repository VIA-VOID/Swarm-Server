#pragma once
#include "Container/RingBuffer.h"

/*-------------------------------------------------------
				SendBuffer
--------------------------------------------------------*/

class SendBuffer : public RingBuffer
{
public:
	SendBuffer(uint32 bufferSize = BUFFER_SIZE);
	virtual ~SendBuffer() {};
	// 데이터 쓰기
	bool Write(const BYTE* src, uint32 size);
	// readPos 이동
	void CommitSend(uint32 size);
	// getter -
	BYTE* GetSendPtr();
	uint32 GetDirectSendSize();

private:
	USE_LOCK;
};
