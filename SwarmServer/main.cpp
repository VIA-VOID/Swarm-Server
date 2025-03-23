#include "pch.h"
#include "GameLogic/AStar.h"
#include "GameLogic/TileMap.h"

#include <iostream>

int wmain()
{
	srand(time(nullptr));
	//srand(20);

	constexpr int32 mapSize = 100;

	TileData* data = new TileData[mapSize * mapSize];

	while (true)
	{
		Pos start = { 0, 0 };
		Pos end = { mapSize - 1, mapSize - 1 };

		int s = 0;
		/*

		for (int i = 0; i < mapSize; i++)
		{
			for (int j = 0; j < mapSize; j++)
			{
				if (i == 1 && j == 2)
				{
					data[s++] = TileData(0, Pos{ i,j });
					continue;
				}

				if (i == 2 && j == 2)
				{
					data[s++] = TileData(0, Pos{ i,j });
					continue;
				}

				if (i == 3 && j == 2)
				{
					data[s++] = TileData(0, Pos{ i,j });
					continue;
				}

				if (i == 4 && j == 2)
				{
					data[s++] = TileData(0, Pos{ i,j });
					continue;
				}
				data[s++] = TileData(1, Pos{ i,j });
			}
		}
			*/



		for (int i = 0; i < mapSize; i++)
		{
			for (int j = 0; j < mapSize; j++)
			{
				if ((i == 0 && j == 0) || (i == mapSize - 1 && j == mapSize - 1))
				{
					data[s++] = TileData(1, Pos{ i, j });
					continue;
				}

				data[s++] = TileData(rand() % 2, Pos{ i, j });
				//data[s++] = TileData(1, Pos{ i, j });
			}
		}

		TileMap* tileMap = TileMap::GetInstance(mapSize);
		tileMap->Init(start, end, data, mapSize * mapSize);

		AStar* aStar = new AStar();
		aStar->Run();

		std::vector<Pos> path = aStar->GetPath();
		std::vector<Pos> allPath = aStar->GetBresenHamPath();
		//std::vector<Pos> allPath = aStar->GetAllPath();

		std::cout << "=========== PATH ===============" << "\n";

		for (auto pos : path)
		{
			std::cout << pos.y << " " << pos.x << "\n";
		}
		std::cout << "=========== ALL PATH =================" << "\n";

		for (auto pos : allPath)
		{
			std::cout << pos.y << " " << pos.x << "\n";
		}
		delete aStar;
	}

	delete[] data;
}
