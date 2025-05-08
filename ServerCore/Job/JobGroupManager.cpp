#include "pch.h"
#include "JobGroupManager.h"

void JobGroupManager::Init()
{
	_nextGroupId.store(JobGroups::NextStart, std::memory_order_relaxed);
	// 기본적인 ServerCore 그룹 생성
	RegisterGroup(JobGroups::System, "System");
	RegisterGroup(JobGroups::Log, "Log", JobPriority::Low);
	RegisterGroup(JobGroups::Network, "Network", JobPriority::High);
}

void JobGroupManager::Shutdown()
{
	// JobGroupType 반환
	for (auto& group : _groups)
	{
		ObjectPool<JobGroupType>::Release(group.second);
	}
	_groups.clear();
}

// 컨텐츠 그룹 등록
JobGroupId JobGroupManager::RegisterContentGroup(const std::string& name, JobPriority priority /*= JobPriority::Normal*/)
{
	// 다음 사용 가능한 컨텐츠 그룹 ID 생성
	JobGroupId newId = _nextGroupId.fetch_add(1);
	return RegisterGroup(newId, name, priority);
}

// 객체 인스턴스 그룹 등록
// 동적 생성용
JobGroupId JobGroupManager::RegisterInstanceGroup(const std::string& baseName, JobPriority priority)
{
	uint32 instanceCounter = ++_instanceGroupCounters[baseName];
	std::string instanceName = baseName + "_" + std::to_string(instanceCounter);
	// 그룹 등록
	JobGroupId newId = _nextGroupId.fetch_add(1);
	JobGroupId groupId = RegisterGroup(newId, instanceName, priority);
	// 등록 후 스레드 생성 요청
	if (groupId != JobGroups::Invalid)
	{
		JobQ.RegisterThreadsForGroup(groupId);
	}
	return groupId;
}

// ID로 그룹 정보 가져오기
const JobGroupType* JobGroupManager::GetGroupInfo(JobGroupId id) const
{
	auto it = _groups.find(id);
	if (it != _groups.end())
	{
		return it->second;
	}
	return nullptr;
}

// 모든 JobGroup 정보 가져오기
const HashMap<JobGroupId, JobGroupType*>& JobGroupManager::GetAllGroups() const
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

// 클래스 타입을 JobGroupId에 매핑 - 매크로에서 사용
void JobGroupManager::SetTypeToGroup(std::type_index idx, JobGroupId groupId)
{
	_typeToGroup[idx] = groupId;
}

// 그룹 등록
JobGroupId JobGroupManager::RegisterGroup(JobGroupId id, const std::string& name, JobPriority priority /*= JobPriority::Normal*/)
{
	// 이미 등록된 ID인지 확인
	if (_groups.find(id) != _groups.end())
	{
		LOG_WARNING(L"이미 등록된 JobGroup ID: " + std::to_wstring(id));
		return JobGroups::Invalid;
	}

	JobGroupType* newGroup = ObjectPool<JobGroupType>::Allocate(id, name, priority);
	_groups[id] = newGroup;
	return id;
}
