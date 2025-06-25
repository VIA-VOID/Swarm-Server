#include "pch.h"
#include "Vector3d.h"

Vector3d::Vector3d()
	: _worldX(0), _worldY(0), _worldZ(0), _worldYaw(0), _gridSize(0)
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

// Grid 좌표로 만들어 전달
void Vector3d::MakeGridIndex(GridIndex& outGridIndex) const
{
	outGridIndex.x = _worldX / (_gridSize * POS_REVISE_NUM);
	outGridIndex.y = _worldY / (_gridSize * POS_REVISE_NUM);
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

void Vector3d::SetWorldX(int32 WorldX)
{
	_worldX = WorldX;
}

void Vector3d::SetWorldY(int32 WorldY)
{
	_worldY = WorldY;
}
