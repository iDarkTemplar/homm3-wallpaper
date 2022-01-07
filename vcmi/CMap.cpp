/*
 * CMap.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include "CMap.h"

#include "CObjectHandler.h"

#include <algorithm>
#include <stdexcept>

CMapHeader::CMapHeader()
	: version(EMapFormat::SOD)
	, height(MAP_SIZE_MIDDLE)
	, width(MAP_SIZE_MIDDLE)
	, twoLevel(false)
{
	players.resize(static_cast<size_t>(PlayerColor::PLAYER_LIMIT_I));
}

CMap::~CMap()
{
	if (terrain)
	{
		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				delete [] terrain[i][j];
			}

			delete [] terrain[i];
		}

		delete [] terrain;
	}

	for (size_t i = 0; i < objects.size(); ++i)
	{
		delete objects[i];
	}
}

TerrainTile& CMap::getTile(const int3 &tile)
{
	if (!isInTheMap(tile))
	{
		throw std::runtime_error("Invalid tile position");
	}

	return terrain[tile.x][tile.y][tile.z];
}

const TerrainTile& CMap::getTile(const int3 &tile) const
{
	if (!isInTheMap(tile))
	{
		throw std::runtime_error("Invalid tile position");
	}

	return terrain[tile.x][tile.y][tile.z];
}

bool CMap::isInTheMap(const int3 &pos) const
{
	if ((pos.x < 0)
		|| (pos.y < 0)
		|| (pos.z < 0)
		|| (pos.x >= width)
		|| (pos.y >= height)
		|| (pos.z > (twoLevel ? 1 : 0)))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CMap::addNewObject(CGObjectInstance *obj)
{
	objects.push_back(obj);
}

void CMap::initTerrain()
{
	int level = twoLevel ? 2 : 1;
	terrain = new TerrainTile**[width];

	for (int i = 0; i < width; ++i)
	{
		terrain[i] = new TerrainTile*[height];

		for (int j = 0; j < height; ++j)
		{
			terrain[i][j] = new TerrainTile[level];
		}
	}
}
