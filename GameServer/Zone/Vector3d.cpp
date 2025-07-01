#include "pch.h"
#include "Vector3d.h"

Vector3d::Vector3d(const ZoneInfo& zoneInfo)
	: _worldX(zoneInfo.worldPos.minX), _worldY(zoneInfo.worldPos.minY),
	_worldZ(0), _worldYaw(0), _gridSize(zoneInfo.gridSize)
{
}

Vector3d::Vector3d(int32 worldX, int32 worldY, int32 gridSize)
	: _worldX(worldX), _worldY(worldY), _worldZ(0), _worldYaw(0), _gridSize(gridSize)
{
}

Vector3d::Vector3d(int32 worldX, int32 worldY, int32 worldZ, int32 worldYaw, int32 gridSize)
	: _worldX(worldX), _worldY(worldY), _worldZ(worldZ), _worldYaw(worldYaw), _gridSize(gridSize)
{
}

// Grid 좌표 만들어 반환
GridIndex Vector3d::MakeGridIndex(const ZonePos& zonePos) const
{
	GridIndex gridIndex;
	gridIndex.x = (_worldX - zonePos.minX) / (_gridSize * POS_REVISE_NUM);
	gridIndex.y = (_worldY - zonePos.minY) / (_gridSize * POS_REVISE_NUM);
	return gridIndex;
}

void Vector3d::operator=(const Vector3d& other)
{
	_worldX = other._worldX;
	_worldY = other._worldY;
	_worldZ = other._worldZ;
	_worldYaw = other._worldYaw;
	_gridSize = other._gridSize;
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

int32 Vector3d::GetWorldX() const
{
	return _worldX;
}

int32 Vector3d::GetWorldY() const
{
	return _worldY;
}

int32 Vector3d::GetWorldZ() const
{
	return _worldZ;
}

int32 Vector3d::GetWorldYaw() const
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
