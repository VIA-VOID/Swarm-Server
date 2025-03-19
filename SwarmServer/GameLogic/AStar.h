#pragma once

#include "Pos.h"

class TileMap;

class AStar
{
public:
	AStar();
	~AStar();

	// 길찾기 실행
	void Run();
	// 최단경로 리턴
	std::vector<Pos>& GetPath();
	// 전체경로 리턴
	std::vector<Pos>& GetAllPath();

private:
	// 휴리스틱 함수(대각선 거리 구하기)
	inline uint32 DiagonalDistance(const Pos& start, const Pos& end);

private:
	TileMap* _tileMap;
	std::vector<Pos> _path;					// A* 최단경로
	std::vector<Pos> _allPath;				// 전체경로(클라 UI용)
	std::map<Pos, Pos> _parent;				// 최단경로 부모 추적
};
