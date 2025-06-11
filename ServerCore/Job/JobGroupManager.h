#pragma once
#include <typeindex>

/*-------------------------------------------------------
				JobGroupManager

- JobGroup 타입 관리
--------------------------------------------------------*/

class JobGroupManager : public Singleton<JobGroupManager>
{
public:
	void Init() override;
	void Shutdown() override;
	// 컨텐츠 그룹 등록
	JobGroupId RegisterContentGroup(const std::string& name, JobPriority priority = JobPriority::Normal);
	// ID로 그룹 정보 가져오기
	const JobGroupTypeRef GetGroupInfo(JobGroupId id) const;
	// 모든 JobGroup 정보 가져오기
	const HashMap<JobGroupId, JobGroupTypeRef>& GetAllGroups() const;
	// ID에 해당하는 이름 가져오기
	const char* GetGroupName(JobGroupId id) const;
	// 클래스 타입을 JobGroupId에 매핑 - 매크로에서 사용
	void SetTypeToGroup(std::type_index idx, JobGroupId groupId);
	// 클래스 타입을 JobGroupId에 매핑
	template <typename T>
	void RegisterTypeMapping(JobGroupId groupId);
	// 타입으로 GroupId 가져오기
	template <typename T>
	JobGroupId GetGroupIdByType() const;

private:
	// 그룹 등록
	JobGroupId RegisterGroup(JobGroupId id, const std::string& name, JobPriority priority = JobPriority::Normal);

private:
	std::atomic<JobGroupId> _nextGroupId;
	HashMap<JobGroupId, JobGroupTypeRef> _groups;
	HashMap<std::type_index, JobGroupId> _typeToGroup;
};

// 클래스 타입을 JobGroupId에 매핑
template<typename T>
inline void JobGroupManager::RegisterTypeMapping(JobGroupId groupId)
{
	std::type_index typeIndex(typeid(T));
	_typeToGroup[typeIndex] = groupId;
}

// 타입으로 GroupId 가져오기
template<typename T>
inline JobGroupId JobGroupManager::GetGroupIdByType() const
{
	std::type_index typeIndex(typeid(T));
	auto it = _typeToGroup.find(typeIndex);
	if (it != _typeToGroup.end())
	{
		return it->second;
	}
	// 기본값으로 시스템 그룹 반환
	return JobGroups::System;
}

template<>
struct std::hash<JobGroupType>
{
	uint64 operator()(const JobGroupType& group) const noexcept
	{
		return group.hash();
	}
};
