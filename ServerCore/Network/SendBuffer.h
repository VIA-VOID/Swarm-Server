#pragma once
#include "Container/RingBuffer.h"

/*-------------------------------------------------------
				SendBuffer
--------------------------------------------------------*/

class SendBuffer : public RingBuffer
{
public:
	SendBuffer(uint32 bufferSize);
	virtual ~SendBuffer() {};
	// 데이터 쓰기
	bool Write(const BYTE* src, uint32 size);
	// readPos 이동
	void CommitSend(uint32 size);
	// readPos 가져오기
	BYTE* GetSendPtr();
	// 전송하지 못한 데이터 크기
	int32 GetRemainSize();

private:
	USE_LOCK;
	int32 _dataSize;
};
