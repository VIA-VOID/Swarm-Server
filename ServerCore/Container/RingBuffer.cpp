#include "pch.h"
#include "RingBuffer.h"

/*----------------------------
		RingBuffer
----------------------------*/

RingBuffer::RingBuffer(uint32 bufferSize /*= BUFFER_SIZE*/)
	: _bufferSize(bufferSize), _readPos(0), _writePos(0)
{
	_buffer.resize(_bufferSize);
}

RingBuffer::~RingBuffer()
{
}

// 사용중인 용량 얻기
uint32 RingBuffer::GetUseSize()
{
	if (_writePos >= _readPos)
	{
		return _writePos - _readPos;
	}
	else
	{
		return _bufferSize - _readPos + _writePos;
	}
}

// 남은 용량 얻기
uint32 RingBuffer::GetFreeSize()
{
	return _bufferSize - GetUseSize();
}

// 끊기지 않고 최대로 넣을 수 있는 길이
uint32 RingBuffer::GetDirectEnqueSize()
{
	if (_readPos > _writePos)
	{
		return _readPos - _writePos;
	}
	else
	{
		return _bufferSize - _writePos;
	}
}

// 끊기지 않고 최대로 뺄수 있는 길이
uint32 RingBuffer::GetDirectDequeSize()
{
	if (_writePos >= _readPos)
	{
		return _writePos - _readPos;
	}
	else
	{
		return _bufferSize - _readPos;
	}
}

// 길이만큼 readPos 이동
void RingBuffer::MoveReadPos(uint32 size)
{
	_readPos = (_readPos + size) % _bufferSize;
}

// 길이만큼 writePos 이동
void RingBuffer::MoveWritePos(uint32 size)
{
	_writePos = (_writePos + size) % _bufferSize;
}

// 길이만큼 데이터 빼오기
// read, write Pos 이동하지 않음
void RingBuffer::Peek(BYTE* dest, uint32 size)
{
	// 앞뒤로 사용 공간이 있으면 두번에 거처 빼내어 합쳐서 전달
	uint32 firstDequeSize = GetDirectDequeSize();
	if (size > firstDequeSize)
	{
		// 나누어 삽입
		::memcpy_s(dest, firstDequeSize, &_buffer[_readPos], firstDequeSize);
		::memcpy_s(dest + firstDequeSize, size - firstDequeSize, &_buffer[0], size - firstDequeSize);
	}
	else
	{
		// 용량이 충분하면 한번에 삽입
		::memcpy_s(dest, size, &_buffer[_readPos], size);
	}
}

// 데이터 삽입
bool RingBuffer::Enqueue(const BYTE* src, uint32 size)
{
	if (size > GetFreeSize())
	{
		return false;
	}
	// 사이즈가 충분
	if (_bufferSize >= _writePos + size)
	{
		::memcpy_s(&_buffer[_writePos], size, src, size);
	}
	else
	{
		// 나누어 삽입
		uint32 firstEnqueSize = GetDirectEnqueSize();
		::memcpy_s(&_buffer[_writePos], firstEnqueSize, src, firstEnqueSize);
		::memcpy_s(&_buffer[0], size - firstEnqueSize, src + firstEnqueSize, size - firstEnqueSize);
	}

	MoveWritePos(size);
	return true;
}

// readPos, writePos가 같을시 위치 초기화
// 나누어 삽입을 최대한 방지하기 위함
void RingBuffer::CleanPos()
{
	uint32 useSize = GetUseSize();
	if (useSize == 0)
	{
		// 0으로 리셋
		_readPos = 0;
		_writePos = 0;
	}
}

BYTE* RingBuffer::GetReadPtr()
{
	return &_buffer[_readPos];
}

BYTE* RingBuffer::GetWritePtr()
{
	return &_buffer[_writePos];
}
