#pragma once
#include "Container/BaseBuffer.h"

/*-------------------------------------------------------
				SendBuffer
--------------------------------------------------------*/

class SendBuffer : public BaseBuffer
{
public:
	SendBuffer(uint32 bufferSize);
	virtual ~SendBuffer() = default;

	// WSASend용 버퍼 정보 반환
	void GetWSABUF(WSABUF& wsabuf);
	// WSASend 완료 후 읽기 위치 이동
	void OnSendCompleted(uint32 bytesTransferred);
	// 송신 완료 여부
	bool IsCompleted() const;
	// 데이터 쓰기
	bool Write(const BYTE* data, uint32 size);
};
