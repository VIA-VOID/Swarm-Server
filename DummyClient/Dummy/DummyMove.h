#pragma once

/*-------------------------------------------------------
				DummyMove

- Dummy 캐릭터 무작위 이동
--------------------------------------------------------*/
class DummyMove : public std::enable_shared_from_this<DummyMove>
{
public:
	DummyMove();
	// 무작위 직선 이동
	void DummyRandomMovement(SessionRef session, Protocol::PosInfo posInfo);

private:
	// 이동 job 실행
	void MoveJob(SessionRef session);
	// 새로 이동
	void NewMove(SessionRef session);
	// Yaw 값 계산
	float CalculateYaw(float dirX, float dirY);

private:
	uint64 _id;
	int32 _intervalTime;
	float _currentX;
	float _currentY;
	float _currentYaw;
	float _targetX;
	float _targetY;
	float _targetYaw;
	float _targetDirX;
	float _targetDirY;
};
