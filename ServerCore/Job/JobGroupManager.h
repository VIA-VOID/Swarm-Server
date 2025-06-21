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
	JobGroupId RegisterContentGroup(const std::string& name);
	// ID로 그룹 정보 가져오기
	const JobGroupTypeRef GetGroupInfo(JobGroupId id) const;
	// 모든 JobGroup 정보 가져오기
	const HashMap<JobGroupId, JobGroupTypeRef>& GetAllGroups() const;
	// ID에 해당하는 이름 가져오기
	const char* GetGroupName(JobGroupId id) const;

private:
	// 그룹 등록
	JobGroupId RegisterGroup(JobGroupId id, const std::string& name);

private:
	std::atomic<JobGroupId> _nextGroupId;
	HashMap<JobGroupId, JobGroupTypeRef> _groups;
};

template<>
struct std::hash<JobGroupType>
{
	uint64 operator()(const JobGroupType& group) const noexcept
	{
		return group.hash();
	}
};
