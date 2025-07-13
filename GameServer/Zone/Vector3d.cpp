#include "pch.h"
#include "Vector3d.h"

Vector3d::Vector3d(const ZoneInfo& zoneInfo)
	: _worldZ(0), _worldYaw(0), _gridSize(zoneInfo.gridSize)
{
	_worldX = static_cast<float>(zoneInfo.worldPos.minX);
	_worldY = static_cast<float>(zoneInfo.worldPos.minY);
}

Vector3d::Vector3d(float worldX, float worldY, int32 gridSize)
	: _worldX(worldX), _worldY(worldY), _worldZ(0), _worldYaw(0), _gridSize(gridSize)
{
}

Vector3d::Vector3d(float worldX, float worldY, float worldZ, float worldYaw, int32 gridSize)
	: _worldX(worldX), _worldY(worldY), _worldZ(worldZ), _worldYaw(worldYaw), _gridSize(gridSize)
{
}

// Grid 좌표 만들어 반환
GridIndex Vector3d::MakeGridIndex(const ZonePos& zonePos) const
{
	GridIndex gridIndex;
	gridIndex.x = static_cast<int32>((_worldX - zonePos.minX) / _gridSize);
	gridIndex.y = static_cast<int32>((_worldY - zonePos.minY) / _gridSize);
	return gridIndex;
}

Vector3d Vector3d::operator+(const Vector3d& other) const
{
	return Vector3d(_worldX + other._worldX, _worldY + other._worldY, _gridSize);
}

Vector3d& Vector3d::operator+=(const Vector3d& other)
{
	(*this) = (*this) + other;
	return *this;
}

Vector3d Vector3d::operator-(const Vector3d& other) const
{
	return Vector3d(_worldX - other._worldX, _worldY - other._worldY, _gridSize);
}

Vector3d& Vector3d::operator-=(const Vector3d& other)
{
	(*this) = (*this) - other;
	return *this;
}

Vector3d Vector3d::operator*(const Vector3d& other) const
{
	return Vector3d(_worldX * other._worldX, _worldY * other._worldY, _gridSize);
}

Vector3d& Vector3d::operator*=(const Vector3d& other)
{
	(*this) = (*this) * other;
	return *this;
}

Vector3d Vector3d::operator/(const Vector3d& other) const
{
	return Vector3d(_worldX / other._worldX, _worldY / other._worldY, _gridSize);
}

Vector3d& Vector3d::operator/=(const Vector3d& other)
{
	(*this) = (*this) / other;
	return *this;
}

float Vector3d::GetWorldX() const
{
	return _worldX;
}

float Vector3d::GetWorldY() const
{
	return _worldY;
}

float Vector3d::GetWorldZ() const
{
	return _worldZ;
}

float Vector3d::GetWorldYaw() const
{
	return _worldYaw;
}

// Protocol::PosInfo의 값으로 멤버 업데이트
void Vector3d::UpdatePosition(const Protocol::PosInfo& InPosInfo)
{
	_worldX = InPosInfo.x();
	_worldY = InPosInfo.y();
	_worldZ = InPosInfo.z();
	_worldYaw = InPosInfo.yaw();
}
