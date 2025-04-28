#pragma once
#include <typeindex>

// Job 우선순위
enum class JobPriority : uint16
{
	Low = 0,
	Normal = 1,
	High = 2,
};

// Job 그룹
enum class JobGroupType : uint16
{
	System = 0,
	Network,
	Log,
	Dungeon,
	Player,
};

// Job 그룹 이름
// - 스레드 이름 지정을 위함
static const char* JobGroupNames[] =
{
	"System",
	"Network",
	"Log",
	"Dungeon",
	"Player",
};

// 그룹별 우선순위
const std::unordered_map<JobGroupType, std::pair<JobPriority, uint16 /*threadCount*/>> GROUP_PRIORITY =
{
	{ JobGroupType::System, { JobPriority::Normal, 1 } },
	{ JobGroupType::Network, { JobPriority::High, 8 } },
	{ JobGroupType::Log, { JobPriority::Low, 1 } },
	{ JobGroupType::Dungeon, { JobPriority::Low, 3 } },
	{ JobGroupType::Player, { JobPriority::High, 3 } },
};

// 타입에 따른 JobGroupType으로 매핑
class TypeToGroupMapper
{
public:
	// JobGroupType의 타입을 추론해서 저장
	// RegisterTypeMapping 함수 사용
	template <typename T>
	static void RegisterType(JobGroupType group)
	{
		std::type_index typeIndex(typeid(T));
		_typeMap[typeIndex] = group;
	}

	// 타입에 맞는 JobGroupType 가져오기
	// 기본값 = System
	template <typename T>
	static JobGroupType GetGroupType()
	{
		std::type_index typeIndex(typeid(T));
		auto it = _typeMap.find(typeIndex);
		if (it != _typeMap.end())
		{
			return it->second;
		}
		// 기본값
		return JobGroupType::System;
	}

private:
	static std::unordered_map<std::type_index, JobGroupType> _typeMap;
};

