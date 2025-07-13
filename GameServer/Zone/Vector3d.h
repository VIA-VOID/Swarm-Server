#pragma once
#include "ZoneDefine.h"

/*--------------------------------------------------------
					Vector3d

- 월드, 그리드 좌표 관리
	- 월드: 클라이언트 좌표
	- 그리드: 서버 좌표
--------------------------------------------------------*/
class Vector3d
{
public:
	Vector3d(const ZoneInfo& zoneInfo);
	Vector3d(float worldX, float worldY, int32 gridSize);
	Vector3d(float worldX, float worldY, float worldZ, float worldYaw, int32 gridSize);
	
	// Grid 좌표 만들어 반환
	GridIndex MakeGridIndex(const ZonePos& zonePos) const;

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
	float GetWorldX() const;
	float GetWorldY() const;
	float GetWorldZ() const;
	float GetWorldYaw() const;
	// Protocol::PosInfo의 값으로 멤버 업데이트
	void UpdatePosition(const Protocol::PosInfo& InPosInfo);

private:
	float _worldX;
	float _worldY;
	float _worldZ;
	float _worldYaw;
	int32 _gridSize;
};
