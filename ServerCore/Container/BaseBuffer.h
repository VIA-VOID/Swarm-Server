#pragma once

/*-------------------------------------------------------
				BaseBuffer

- Send, Recv Buffer 부모 클래스
--------------------------------------------------------*/

class BaseBuffer
{
public:
	BaseBuffer(uint32 bufferSize);
	virtual ~BaseBuffer() = default;

	// 읽기 위치 이동
	void MoveReadPos(uint32 size);
	// 쓰기 위치 이동
	void MoveWritePos(uint32 size);
	// 버퍼 정리
	// 사용하지 않는 앞쪽 공간 제거
	void Compact();

	// Getter-
	uint32 GetReadPos() const;
	uint32 GetWritePos() const;
	uint32 GetBufferSize() const;
	uint32 GetDataSize() const;
	uint32 GetFreeSize() const;
	
	BYTE* GetBufferStart();
	BYTE* GetReadPtr();
	BYTE* GetWritePtr();

protected:
	Vector<BYTE> _buffer;
	uint32 _bufferSize;
	uint32 _readPos;
	uint32 _writePos;
};
