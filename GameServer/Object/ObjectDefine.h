#pragma once
#include "pch/Types.h"
#include <atomic>

/*-------------------------------------------------------
				ObjectId

- Object의 고유 Id
--------------------------------------------------------*/
class ObjectId
{
public:
	ObjectId() : _id(0) {}
	ObjectId(uint64 id) : _id(id) {}
	uint64 GetId() const
	{
		return _id;
	}
	static ObjectId Generate()
	{
		static std::atomic<uint64> atomicObjectId = 1;
		return ObjectId(atomicObjectId.fetch_add(1, std::memory_order_relaxed));
	}
	bool operator==(const ObjectId& other) const { return _id == other._id; }
	bool operator!=(const ObjectId& other) const { return _id != other._id; }

private:
	uint64 _id;
};

template<>
struct std::hash<ObjectId>
{
	uint64 operator()(const ObjectId& id) const noexcept
	{
		return std::hash<uint64>()(id.GetId());
	}
};