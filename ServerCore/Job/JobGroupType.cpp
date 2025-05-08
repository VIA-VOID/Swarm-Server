#include "pch.h"
#include "JobGroupType.h"

/*----------------------------
		JobGroupType
----------------------------*/

JobGroupType::JobGroupType(JobGroupId id, const std::string& name, JobPriority priority)
	: _id(id), _name(name), _priority(priority)
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

JobPriority JobGroupType::GetGroupPriority() const
{
	return _priority;
}

bool JobGroupType::operator==(const JobGroupType& other) const
{
	return _id == other._id;
}

bool JobGroupType::operator!=(const JobGroupType& other) const
{
	return _id != other._id;
}
