#pragma once
#include "ZoneDefine.h"

/*--------------------------------------------------------
					Vector3d

- 월드, 그리드 좌표 관리
	- 월드: 클라이언트 좌표
	- 그리드: 서버 좌표
- 고정 소수점 연산 캡슐화
	- 클라이언트 월드 좌표 = 소수점 3자리
	- 서버에서 Int로 보정하여 관리한다.
--------------------------------------------------------*/
class Vector3d
{
public:
	Vector3d();
	Vector3d(int32 worldX, int32 worldY, int32 gridSize);
	Vector3d(int32 worldX, int32 worldY, int32 worldZ, int32 worldYaw, int32 gridSize);
	
	// Grid 좌표로 만들어 전달
	void MakeGridIndex(GridIndex& outGridIndex) const;

	// 연산자 오버로딩
	Vector3d operator+(const Vector3d& other) const;
	Vector3d& operator+=(const Vector3d& other);
	Vector3d operator-(const Vector3d& other) const;
	Vector3d& operator-=(const Vector3d& other);
	Vector3d operator*(const Vector3d& other) const;
	Vector3d& operator*=(const Vector3d& other);
	Vector3d operator/(const Vector3d& other) const;
	Vector3d& operator/=(const Vector3d& other);
	
	// Getter-
	int32 GetWorldX() const;
	int32 GetWorldY() const;
	void SetWorldX(int32 WorldX);
	void SetWorldY(int32 WorldY);

private:
	int32 _worldX;
	int32 _worldY;
	int32 _worldZ;
	int32 _worldYaw;
	int32 _gridSize;
};
