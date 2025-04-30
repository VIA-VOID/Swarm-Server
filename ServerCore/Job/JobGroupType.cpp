#include "pch.h"
#include "JobGroupType.h"

/*----------------------------
		JobGroupType
----------------------------*/

JobGroupType::JobGroupType(JobGroupId id, const std::string& name, uint16 threadCount, bool isInit, JobPriority priority)
	: _id(id), _name(name), _threadCount(threadCount), _isInit(true), _priority(priority)
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

uint16 JobGroupType::GetGroupThreadCount() const
{
	return _threadCount;
}

JobPriority JobGroupType::GetGroupPriority() const
{
	return _priority;
}

bool JobGroupType::GetGroupIsInit() const
{
	return _isInit;
}

bool JobGroupType::operator==(const JobGroupType& other) const
{
	return _id == other._id;
}

bool JobGroupType::operator!=(const JobGroupType& other) const
{
	return _id != other._id;
}
