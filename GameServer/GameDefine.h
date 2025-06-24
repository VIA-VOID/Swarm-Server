#pragma once

// 업데이트 주기(프레임)
constexpr std::chrono::milliseconds FRAME_INTERVAL = std::chrono::milliseconds(50);
// 좌표 보정 수치(기본 소수점 3자리)
constexpr int32 POS_REVISE_NUM = 1000;
// Player 시야 범위
constexpr uint16 PLAYER_VISUAL_RANGE = 100;