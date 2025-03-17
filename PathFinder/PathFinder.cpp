#pragma once

#include "pch.h"
#include "TileMap.h"
#include "AStar.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)

DLL_EXPORT void InitTileMap(int mapSize, Pos start, Pos end, TileData* tiles, int tileSize)
{
	TileMap* tileMap = TileMap::GetInstance(mapSize);
	tileMap->Init(start, end, tiles, 9);
}

DLL_EXPORT void RunPathFind(Pos** outPath, int* outPathSize, Pos** outAllPath, int* outAllPathSize)
{
	AStar* aStar = new AStar();
	aStar->Run();

	std::vector<Pos> path = aStar->GetPath();
	std::vector<Pos> allPath = aStar->GetAllPath();

	*outPathSize = static_cast<int>(path.size());
	*outAllPathSize = static_cast<int>(allPath.size());

	Pos* pathArray = new Pos[*outPathSize];
	Pos* allPathArray = new Pos[*outAllPathSize];

	if (*outPathSize > 0)
	{
		std::copy(path.begin(), path.end(), pathArray);
	}

	if (*outAllPathSize > 0)
	{
		std::copy(allPath.begin(), allPath.end(), allPathArray);
	}

	*outPath = pathArray;
	*outAllPath = allPathArray;

	delete aStar;
}

DLL_EXPORT void FreePathArray(Pos* arr)
{
	delete[] arr;
}
