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
};
