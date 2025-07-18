#pragma once
#include "ZoneDefine.h"

/*--------------------------------------------------------
					Vector3d

- 월드좌표 관리
--------------------------------------------------------*/
class Vector3d
{
public:
	Vector3d(float worldX, float worldY);
	Vector3d(float worldX, float worldY, float worldZ, float worldYaw);

	// 연산자 오버로딩
	Vector3d operator+(const Vector3d& other) const;
	Vector3d& operator+=(const Vector3d& other);
	Vector3d operator-(const Vector3d& other) const;
	Vector3d& operator-=(const Vector3d& other);
	Vector3d operator*(const Vector3d& other) const;
	Vector3d& operator*=(const Vector3d& other);
	Vector3d operator/(const Vector3d& other) const;
	Vector3d& operator/=(const Vector3d& other);

	// Protocol::PosInfo의 값으로 멤버 업데이트
	void UpdatePosition(const Protocol::PosInfo& InPosInfo);
	// Protocol::PosInfo 변환후 반환
	Protocol::PosInfo MakePosInfo() const;

public:
	float x;
	float y;
	float z;
	float yaw;
};
