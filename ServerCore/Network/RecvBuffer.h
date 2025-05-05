#pragma once
#include "Container/RingBuffer.h"

/*-------------------------------------------------------
				RecvBuffer
--------------------------------------------------------*/

class RecvBuffer : public RingBuffer
{
public:
	RecvBuffer(uint32 bufferSize);
	virtual ~RecvBuffer() {};
	// 데이터 읽기
	bool Read(BYTE* dest, uint32 destSize, uint32 size);
	// writePos 이동
	void CommitRecv(uint32 size);
	// 패킷 크기 만큼의 연속 공간이 있는지 확인
	bool CanRecvPacketSize(uint32 packetSize);
	// getter -
	BYTE* GetRecvPtr();
	uint32 GetDirectRecvSize();
};
