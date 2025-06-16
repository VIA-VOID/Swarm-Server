#include "pch.h"
#include "JobGroupManager.h"

void JobGroupManager::Init()
{
	_nextGroupId.store(JobGroups::NextStart, std::memory_order_relaxed);
	// 기본적인 ServerCore 그룹 생성
	RegisterGroup(JobGroups::System, "System");
	RegisterGroup(JobGroups::Network, "Network", JobPriority::High);
}

void JobGroupManager::Shutdown()
{
	_groups.clear();
}

// 컨텐츠 그룹 등록
JobGroupId JobGroupManager::RegisterContentGroup(const std::string& name, JobPriority priority /*= JobPriority::Normal*/)
{
	// 다음 사용 가능한 컨텐츠 그룹 ID 생성
	JobGroupId newId = _nextGroupId.fetch_add(1);
	return RegisterGroup(newId, name, priority);
}

// ID로 그룹 정보 가져오기
const JobGroupTypeRef JobGroupManager::GetGroupInfo(JobGroupId id) const
{
	auto it = _groups.find(id);
	if (it != _groups.end())
	{
		return it->second;
	}
	return nullptr;
}

// 모든 JobGroup 정보 가져오기
const HashMap<JobGroupId, JobGroupTypeRef>& JobGroupManager::GetAllGroups() const
{
	return _groups;
}

// ID에 해당하는 이름 가져오기
const char* JobGroupManager::GetGroupName(JobGroupId id) const
{
	auto it = _groups.find(id);
	if (it != _groups.end() && it->second != nullptr)
	{
		return it->second->GetGroupName().c_str();
	}
	return "System";
}

// 그룹 등록
JobGroupId JobGroupManager::RegisterGroup(JobGroupId id, const std::string& name, JobPriority priority /*= JobPriority::Normal*/)
{
	// 이미 등록된 ID인지 확인
	if (_groups.find(id) != _groups.end())
	{
		LOG_WARNING("이미 등록된 JobGroup ID: " + std::to_string(id));
		return JobGroups::Invalid;
	}

	JobGroupTypeRef newGroup = ObjectPool<JobGroupType>::MakeShared(id, name, priority);
	_groups[id] = newGroup;
	return id;
}
