#include "pch.h"
#include "JobGroupType.h"

/*----------------------------
		JobGroupType
----------------------------*/

JobGroupType::JobGroupType(JobGroupId id, const std::string& name)
	: _id(id), _name(name)
{
}

JobGroupId JobGroupType::GetGroupId() const
{
	return _id;
}

const std::string& JobGroupType::GetGroupName() const
{
	return _name;
}

bool JobGroupType::operator==(const JobGroupType& other) const
{
	return _id == other._id;
}

bool JobGroupType::operator!=(const JobGroupType& other) const
{
	return _id != other._id;
}
