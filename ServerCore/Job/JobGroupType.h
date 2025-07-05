#pragma once
#include "Pch/Types.h"

// Job 그룹
namespace JobGroups
{
	constexpr JobGroupId Invalid = UINT16_MAX;
	constexpr JobGroupId System = 0;
	constexpr JobGroupId Network = 1;
	constexpr JobGroupId NextStart = 10;
}

/*-------------------------------------------------------
				JobGroupType
--------------------------------------------------------*/

class JobGroupType
{
public:
	JobGroupType(JobGroupId id, const std::string& name);

	// Getter-
	JobGroupId GetGroupId() const;
	const std::string& GetGroupName() const;

	// operator-
	bool operator==(const JobGroupType& other) const;
	bool operator!=(const JobGroupType& other) const;

	// 해시함수
	uint64 hash() const
	{
		return std::hash<JobGroupId>()(_id);
	}

private:
	JobGroupId _id;         // 고유 ID
	std::string _name;      // 그룹 이름(스레드 이름으로 사용)
};
