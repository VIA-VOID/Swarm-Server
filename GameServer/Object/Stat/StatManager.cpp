#include "pch.h"
#include "StatManager.h"
#include "PlayerStatLoader.h"

void StatManager::Init()
{
	// 스텟정보 읽기
	std::string dir = "Data/PlayerStats.json";
	if (PlayerStatLoader::LoadPlayerStat(dir, _playerStats) == false)
	{
		CRASH("플레이어 스텟 읽기 실패");
		return;
	}
}

void StatManager::Shutdown()
{
	_playerStats.clear();
}

// 직업에 맞는 플레이어 스텟 가져오기
void StatManager::GetPlayerStat(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo)
{
	GetBaseStatInfo(playerType, outStatInfo);
}

// 레벨업시 성장 스텟 계산
void StatManager::CalculatePlayerStat(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo)
{
	// 성장 스텟 가져오기
	Protocol::StatInfo statGrowth;
	GetBaseStatInfo(playerType, statGrowth);
	// 계산
	outStatInfo += statGrowth;
}

// 직업에 맞는 BaseStat 가져오기
void StatManager::FindBaseState(const Protocol::PlayerType playerType, BaseStat& outBaseStat)
{
	auto findIt = _playerStats.find(playerType);
	if (findIt != _playerStats.end())
	{
		outBaseStat = findIt->second;
	}
}

// StatInfo 가져오기
void StatManager::GetBaseStatInfo(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo)
{
	BaseStat baseStat;
	FindBaseState(playerType, baseStat);
	baseStat.GetStatInfo(outStatInfo);
}

// StatGrowth 가져오기
void StatManager::GetBaseStatGrowth(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatGrowth)
{
	BaseStat baseStat;
	FindBaseState(playerType, baseStat);
	baseStat.GetStatGrowth(outStatGrowth);
}
