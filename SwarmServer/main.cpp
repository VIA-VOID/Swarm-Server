#include "pch.h"
#include "GameLogic/AStar.h"
#include "GameLogic/TileMap.h"

#include <iostream>

int wmain()
{
	srand(time(nullptr));


	constexpr int32 mapSize = 100;

	TileData* data = new TileData[mapSize * mapSize];


	while (true)
	{
		Pos start = { 0, 0 };
		Pos end = { mapSize - 1, mapSize - 1 };


		int s = 0;

		for (int i = 0; i < mapSize; i++)
		{
			for (int j = 0; j < mapSize; j++)
			{
				data[s++] = TileData(rand() % 2, Pos{ i, j });
			}
		}

		TileMap* tileMap = TileMap::GetInstance(mapSize);
		tileMap->Init(start, end, data, mapSize * mapSize);

		AStar* aStar = new AStar();
		aStar->Run();

		std::vector<Pos> path = aStar->GetPath();
		std::vector<Pos> allPath = aStar->GetAllPath();
		for (auto pos : path)
		{
			std::cout << pos.y << " " << pos.x << "\n";
		}

		std::cout << "==============================" << "\n";

		for (auto pos : allPath)
		{
			std::cout << pos.y << " " << pos.x << "\n";
		}
		delete aStar;
	}

	delete[] data;


}
