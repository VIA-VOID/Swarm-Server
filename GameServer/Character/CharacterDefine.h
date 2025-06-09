#pragma once
#include "pch/Types.h"

/*-------------------------------------------------------
				CharacterId

- CharacterObject의 고유 Id
--------------------------------------------------------*/
class CharacterId
{
public:
	CharacterId() : _id(0) {}
	CharacterId(uint64 id) : _id(id) {}
	uint64 GetId() const
	{
		return _id;
	}
	static CharacterId Generate()
	{
		static std::atomic<uint64> atomicCharacterId = 1;
		return CharacterId(atomicCharacterId.fetch_add(1, std::memory_order_relaxed));
	}
	bool operator==(const CharacterId& other) const { return _id == other._id; }
	bool operator!=(const CharacterId& other) const { return _id != other._id; }

private:
	uint64 _id;
};

template<>
struct std::hash<CharacterId>
{
	uint64 operator()(const CharacterId& id) const noexcept
	{
		return std::hash<uint64>()(id.GetId());
	}
};