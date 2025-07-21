#include "pch.h"
#include "DummyMove.h"
#include "Utils/Utils.h"
#include "Packet/PacketId.h"
#include "Packet/PacketHandler.h"

constexpr float speed = 500.f;
constexpr float deltaTime = 0.1f;
constexpr int32 minX = 18000;
constexpr int32 minY = 32000;
constexpr int32 maxX = 28000;
constexpr int32 maxY = 41000;

DummyMove::DummyMove()
{
	_intervalTime = Utils::GetRandom<int32>(80, 120);
}

// 무작위 직선 이동
void DummyMove::DummyRandomMovement(SessionRef session, Protocol::PosInfo posInfo)
{
	_id = session->GetSessionID().GetID();
	_currentX = posInfo.x();
	_currentY = posInfo.y();
	_currentYaw = posInfo.yaw();

	_targetX = static_cast<float>(Utils::GetRandom<int32>(minX, maxX));
	_targetY = static_cast<float>(Utils::GetRandom<int32>(minY, maxY));

	// 목표지점까지의 yaw 구하기
	float dirX = _targetX - _currentX;
	float dirY = _targetY - _currentY;
	float distance = std::sqrt(dirX * dirX + dirY * dirY);

	if (distance <= 0.01f)
	{
		LOG_WARNING("너무짧은 목표거리, 재지정, id: " + std::to_string(_id));
		DummyRandomMovement(session, posInfo);
		return;
	}
	
	_targetDirX = dirX / distance;
	_targetDirY = dirY / distance;
	_targetYaw = CalculateYaw(_targetDirX, _targetDirY);

	int32 startDelay = Utils::GetRandom<int32>(0, 1000);
	JobQ.DoAsyncAfter(startDelay, [this, session]() {
		MoveJob(session);
	}, JobGroup::Timer);
}

// 이동 job 실행
void DummyMove::MoveJob(SessionRef session)
{
	float dirX = _targetX - _currentX;
	float dirY = _targetY - _currentY;
	float distance = std::sqrt(dirX * dirX + dirY * dirY);

	// 이동완료
	if (distance <= 10.f)
	{
		LOG_INFO("이동완료, id: " + std::to_string(_id));
		
		// 새로운 이동 시작
		NewMove(session);
		return;
	}

	_currentX += _targetDirX * speed * deltaTime;
	_currentY += _targetDirY * speed * deltaTime;
	_currentYaw = _targetYaw;

	// 이동범위 초과
	// distance 허용범위가 너무 크면, 다음 이동시 캐릭터가 순간이동해버리고
	// 허용범위가 너무 작으면 min or max 지점까지 끝까지 가버림
	if (_currentX < minX || _currentX > maxX || _currentY < minY || _currentY > maxY)
	{
		// 새로운 이동 시작
		LOG_WARNING("이동범위초과, 목표 재지정, id: " + std::to_string(_id));
		NewMove(session);
		return;
	}

	// 이동패킷 전송
	Protocol::CS_PLAYER_MOVE movePkt;
	Protocol::PosInfo currentPos;
	Protocol::PosMoveVector moveVector;
	currentPos.set_x(_currentX);
	currentPos.set_y(_currentY);
	currentPos.set_z(180.f);
	currentPos.set_yaw(_currentYaw);
	moveVector.set_x(_targetDirX);
	moveVector.set_y(_targetDirY);

	//LOG_INFO(std::to_string(session->GetSessionID().GetID()) + " 위치 x: " + std::to_string(_currentX) + ", y: " + std::to_string(_currentY));

	movePkt.mutable_posinfo()->CopyFrom(currentPos);
	movePkt.mutable_movevector()->CopyFrom(moveVector);
	PacketHandler::SendPacket(session, movePkt, PacketID::CS_PLAYER_MOVE);

	JobQ.DoAsyncAfter(_intervalTime, [this, session]() {
		MoveJob(session);
	}, JobGroup::Timer);
}

// 새로 이동
void DummyMove::NewMove(SessionRef session)
{
	Protocol::PosInfo targetPos;
	targetPos.set_x(_currentX);
	targetPos.set_y(_currentY);
	targetPos.set_z(180.f);
	targetPos.set_yaw(_currentYaw);

	JobQ.DoAsyncAfter(_intervalTime * 10, [this, session, targetPos]() {
		DummyRandomMovement(session, targetPos);
	}, JobGroup::Timer);
}

// Yaw 값 계산
float DummyMove::CalculateYaw(float dirX, float dirY)
{
	float radian = std::atan2(dirY, dirX);
	return radian * (180.0f / 3.141592f);
}
