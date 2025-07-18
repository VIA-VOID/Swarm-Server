#include "pch.h"
#include "Vector3d.h"

Vector3d::Vector3d(float worldX, float worldY)
	: x(worldX), y(worldY), z(0), yaw(0)
{
}

Vector3d::Vector3d(float worldX, float worldY, float worldZ, float worldYaw)
	: x(worldX), y(worldY), z(worldZ), yaw(worldYaw)
{
}

Vector3d Vector3d::operator+(const Vector3d& other) const
{
	return Vector3d(x + other.x, y + other.y);
}

Vector3d& Vector3d::operator+=(const Vector3d& other)
{
	(*this) = (*this) + other;
	return *this;
}

Vector3d Vector3d::operator-(const Vector3d& other) const
{
	return Vector3d(x - other.x, y - other.y);
}

Vector3d& Vector3d::operator-=(const Vector3d& other)
{
	(*this) = (*this) - other;
	return *this;
}

Vector3d Vector3d::operator*(const Vector3d& other) const
{
	return Vector3d(x * other.x, y * other.y);
}

Vector3d& Vector3d::operator*=(const Vector3d& other)
{
	(*this) = (*this) * other;
	return *this;
}

Vector3d Vector3d::operator/(const Vector3d& other) const
{
	return Vector3d(x / other.x, y / other.y);
}

Vector3d& Vector3d::operator/=(const Vector3d& other)
{
	(*this) = (*this) / other;
	return *this;
}

// Protocol::PosInfo의 값으로 멤버 업데이트
void Vector3d::UpdatePosition(const Protocol::PosInfo& InPosInfo)
{
	x = InPosInfo.x();
	y = InPosInfo.y();
	z = InPosInfo.z();
	yaw = InPosInfo.yaw();
}

// Protocol::PosInfo 변환후 반환
Protocol::PosInfo Vector3d::MakePosInfo() const
{
	Protocol::PosInfo posInfo;
	posInfo.set_x(x);
	posInfo.set_y(y);
	posInfo.set_z(z);
	posInfo.set_yaw(yaw);
	return posInfo;
}
