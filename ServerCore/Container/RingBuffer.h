#pragma once

/*-------------------------------------------------------
				RingBuffer

- Send, Recv Buffer 부모 클래스
- 순환버퍼
--------------------------------------------------------*/

class RingBuffer
{
public:
	RingBuffer(uint32 bufferSize);
	virtual ~RingBuffer();

	// 사용중인 용량 얻기
	uint32 GetUseSize();
	// 남은 용량 얻기
	uint32 GetFreeSize();
	// readPos, writePos가 같을시 위치 초기화
	// 나누어 삽입을 최대한 방지하기 위함
	void CleanPos();
	// 시작지점 포인터 반환
	BYTE* GetBufferStart();
	// 길이만큼 데이터 빼오기
	// read, write Pos 이동하지 않음
	void Peek(BYTE* dest, uint32 destSize, uint32 size);

protected:
	// 끊기지 않고 최대로 넣을 수 있는 길이
	uint32 GetDirectEnqueSize();
	// 끊기지 않고 최대로 뺄수 있는 길이
	uint32 GetDirectDequeSize();
	// 길이만큼 readPos 이동
	void MoveReadPos(uint32 size);
	// 길이만큼 writePos 이동
	void MoveWritePos(uint32 size);
	// 데이터 삽입
	bool Enqueue(const BYTE* src, uint32 size);

	// Getter -
	BYTE* GetReadPtr();
	BYTE* GetWritePtr();

protected:
	Vector<BYTE> _buffer;
	uint32 _bufferSize;
	uint32 _readPos;
	uint32 _writePos;
};
