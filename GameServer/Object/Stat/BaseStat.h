#pragma once

/*-------------------------------------------------------
					BaseStat

- GameObject 파생 클래스에서 공통으로 사용되는 스탯 데이터
--------------------------------------------------------*/

class BaseStat
{
public:
	// 레벨업시 스텟 계산
	void CalculateLevelUpStat(Protocol::StatInfo& currentStat) const;

	// Setter-
	void SetStatInfo(const Protocol::StatInfo& statInfo);
	void SetStatGrowth(const Protocol::StatInfo& statGrowth);

private:
	// 기본스텟 정보
	Protocol::StatInfo _statInfo;
	// 성장스텟 수치 (레벨업시)
	Protocol::StatInfo _statGrowth;
};
