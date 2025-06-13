#include "pch.h"
#include "SendBuffer.h"

/*----------------------------
		SendBuffer
----------------------------*/

SendBuffer::SendBuffer(uint32 bufferSize/* = BUFFER_SIZE*/)
	: RingBuffer(bufferSize)
{
}
