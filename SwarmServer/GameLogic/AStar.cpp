#include "pch.h"
#include "AStar.h"
#include "TileMap.h"

// 가중치
constexpr uint16 MOVENUM = 8;	// 8방 이동
constexpr uint16 STRAIGHT = 10;	// 직선
constexpr uint16 DIAGONAL = 14;	// 대각선

Pos moveTo[MOVENUM] =
{
	Pos {-1, 0 }, // 위
	Pos { 0,-1 }, // 왼쪽
	Pos { 1, 0 }, // 아래
	Pos { 0, 1 }, // 오른쪽
	Pos {-1, -1}, // 왼쪽 위 대각선
	Pos { 1, -1}, // 왼쪽 아래 대각선
	Pos { 1, 1 }, // 오른쪽 아래 대각선
	Pos {-1, 1 }, // 오른쪽 위 대각선
};

uint16 cost[MOVENUM] =
{
	STRAIGHT, // 위
	STRAIGHT, // 왼쪽
	STRAIGHT, // 아래
	STRAIGHT, // 오른쪽
	DIAGONAL, // 왼쪽 위 대각선
	DIAGONAL, // 왼쪽 아래 대각선
	DIAGONAL, // 오른쪽 아래 대각선
	DIAGONAL, // 오른쪽 위 대각선
};

AStar::AStar()
{
	_tileMap = TileMap::GetInstance();
}

AStar::~AStar()
{
}

// 휴리스틱 함수(대각선 거리 구하기)
inline uint32 AStar::DiagonalDistance(const Pos& start, const Pos& end)
{
	return DIAGONAL * max(abs(end.x - start.x), abs(end.y - start.y));
}

// 길찾기 실행
void AStar::Run()
{
	int32 mapSize = _tileMap->GetMapSize();

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> _openListPQ;
	std::vector<std::vector<int32>> _best = std::vector<std::vector<int32>>(mapSize, std::vector<int32>(mapSize, INT_MAX));
	std::vector<std::vector<bool>> _closeList = std::vector<std::vector<bool>>(mapSize, std::vector<bool>(mapSize, false));

	_path.clear();
	_allPath.clear();
	_parent.clear();

	// 필요값 초기화
	bool isFind = false;
	const Pos start = _tileMap->GetStartPos();
	const Pos end = _tileMap->GetEndPos();
	int32 g = 0;
	int32 h = DiagonalDistance(start, end);

	_closeList[start.y][start.x] = true;
	_best[start.y][start.x] = g + h;
	_parent[start] = start;
	_openListPQ.push(PQNode{ g + h, g, start });

	// 길찾기 시작
	while (_openListPQ.empty() == false)
	{
		if (isFind) break;
		PQNode node = _openListPQ.top();
		_openListPQ.pop();

		// 만일 최적경로가 있었다면 스킵
		if (_best[node.pos.y][node.pos.x] < node.f)
		{
			continue;
		}

		for (int32 dir = 0; dir < MOVENUM; dir++)
		{
			Pos nextPos = node.pos + moveTo[dir];
			// 목적지에 도달했다면 종료
			if (nextPos == end)
			{
				_parent[nextPos] = node.pos;
				isFind = true;
				break;
			}
			// 갈수없다면 스킵
			if (_tileMap->CanGo(nextPos) == false)
			{
				continue;
			}
			// 이미 방문한곳이면 스킵
			if (_closeList[nextPos.y][nextPos.x] == true)
			{
				continue;
			}
			// UI용 path
			_allPath.push_back(nextPos);
			// 비용 계산
			int32 g = node.g + cost[dir];
			int32 h = DiagonalDistance(nextPos, end);
			int32 f = g + h;
			// 더 빠른 길을 찾았다면 스킵
			if (_best[nextPos.y][nextPos.x] <= f)
			{
				continue;
			}
			// 방문처리
			_closeList[nextPos.y][nextPos.x] = true;
			_best[nextPos.y][nextPos.x] = f;
			_parent[nextPos] = node.pos;
			_openListPQ.push(PQNode{ g + h, g, nextPos });
		}
	}

	// 길찾기 완료 후 전체 경로 삽입
	Pos pos;
	if (isFind == false)
	{
		// 만약 길이 모두 막혀있을 경우
		int64 allPathSize = static_cast<int64>(_allPath.size());
		if (allPathSize == 0)
		{
			return;
		}
		pos = _allPath[allPathSize - 1];
	}
	else
	{
		// 도착지 까지 길을 모두 찾았다면
		pos = end;
	}

	while (true)
	{
		_path.push_back(pos);
		// 시작점까지
		if (pos == _parent[pos])
		{
			break;
		}
		pos = _parent[pos];
	}

	std::reverse(_path.begin(), _path.end());
}

// 최단경로 리턴
std::vector<Pos>& AStar::GetPath()
{
	return _path;
}

// 전체경로 리턴
std::vector<Pos>& AStar::GetAllPath()
{
	return _allPath;
}
