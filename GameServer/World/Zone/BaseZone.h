#pragma once

/*-------------------------------------------------------
				BaseZone

- 모든 Zone의 기본 클래스
--------------------------------------------------------*/
class BaseZone : public std::enable_shared_from_this<BaseZone>
{
public:
	BaseZone(const ZoneInfo& zoneInfo, uint64 updateTime = FRAME_INTERVAL.count());
	virtual ~BaseZone();

	// Zone 업데이트, 스레드에서 실행
	virtual void ZoneUpdate() = 0;
	// Zone World 좌표 가져오기
	ZonePos GetWorldPosition() const;
	// spawnOffset 설정
	void SetSpawnOffset(int32 offset);

protected:
	int32 _spawnOffset;
	ZoneInfo _zoneInfo;
	uint64 _updateTime;
};
