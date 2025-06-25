#pragma once

#include "PlayerStatLoader.h"

/*-------------------------------------------------------
				StatManager

- GameObject 스탯 데이터 관리
--------------------------------------------------------*/
class StatManager : public Singleton<StatManager>
{
public:
	void Init() override;
	void Shutdown() override;

	// 직업에 맞는 플레이어 스텟 가져오기
	void GetPlayerStat(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo);
	// 레벨업시 성장 스텟 계산
	void CalculatePlayerStat(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo);

private:
	// BaseStat 가져오기
	void FindBaseState(const Protocol::PlayerType playerType, BaseStat& outBaseStat);
	// StatInfo 가져오기
	void GetBaseStatInfo(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatInfo);
	// StatGrowth 가져오기
	void GetBaseStatGrowth(const Protocol::PlayerType playerType, Protocol::StatInfo& outStatGrowth);

private:
	// HashMap<Protocol::PlayerType, BaseStat>
	PlayerStatMap _playerStats;
};
