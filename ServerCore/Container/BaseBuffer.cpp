#include "pch.h"
#include "BaseBuffer.h"

/*----------------------------
		BaseBuffer
----------------------------*/

BaseBuffer::BaseBuffer(uint32 bufferSize)
	: _bufferSize(bufferSize), _readPos(0), _writePos(0)
{
	_buffer.resize(_bufferSize);
}

// 읽기 위치 이동
void BaseBuffer::MoveReadPos(uint32 size)
{
	ASSERT_CRASH(_readPos + size <= _writePos);
	_readPos += size;
}

// 쓰기 위치 이동
void BaseBuffer::MoveWritePos(uint32 size)
{
	ASSERT_CRASH(_writePos + size <= _bufferSize);
	_writePos += size;
}

// 버퍼 정리
// 사용하지 않는 앞쪽 공간 제거
void BaseBuffer::Compact()
{
	if (_readPos == 0)
	{
		return;
	}
	// 데이터가 없으면 모든 위치 초기화
	uint32 dataSize = GetDataSize();
	if (dataSize == 0)
	{
		_readPos = 0;
		_writePos = 0;
	}
	else
	{
		// 여유공간이 얼마 남지 않았다면 남은 데이터를 앞쪽으로 이동
		if (GetFreeSize() < MAX_PACKET_SIZE)
		{
			::memmove(_buffer.data(), _buffer.data() + _readPos, dataSize);
			_writePos = dataSize;
			_readPos = 0;
		}
	}
}

uint32 BaseBuffer::GetReadPos() const
{
	return _readPos;
}

uint32 BaseBuffer::GetWritePos() const
{
	return _writePos;
}

uint32 BaseBuffer::GetBufferSize() const
{
	return _bufferSize;
}

uint32 BaseBuffer::GetDataSize() const
{
	return _writePos - _readPos;
}

uint32 BaseBuffer::GetFreeSize() const
{
	return _bufferSize - _writePos;
}

BYTE* BaseBuffer::GetBufferStart()
{
	return _buffer.data();
}

BYTE* BaseBuffer::GetReadPtr()
{
	return _buffer.data() + _readPos;
}

BYTE* BaseBuffer::GetWritePtr()
{
	return _buffer.data() + _writePos;
}
