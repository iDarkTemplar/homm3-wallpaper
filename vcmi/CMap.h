/*
 * CMap.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "int3.h"
#include "GameConstants.h"
#include "CMapDefines.h"

#include <vector>

class CGObjectInstance;

/// The player info constains data about which factions are allowed, AI tactical settings,
/// the main hero name, where to generate the hero, whether the faction should be selected randomly,...
struct PlayerInfo
{
	PlayerInfo() = default;

	std::vector<ETownType> allowedFactions;
	bool isFactionActive = false;
	bool isFactionRandom = false;
	ETownType playerFaction = ETownType::ANY;

	bool hasMainTown = false;
	bool generateHeroAtMainTown = false;
	int3 posOfMainTown = int3(-1);
};

enum class EMapFormat: uint8_t
{
	INVALID = 0,
	ROE = 0x0e, // 14
	AB  = 0x15, // 21
	SOD = 0x1c, // 28
};

/// The map header holds information about loss/victory condition,map format, version, players, height, width,...
class CMapHeader
{
public:
	static const int MAP_SIZE_SMALL = 36;
	static const int MAP_SIZE_MIDDLE = 72;
	static const int MAP_SIZE_LARGE = 108;
	static const int MAP_SIZE_XLARGE = 144;

	CMapHeader();
	virtual ~CMapHeader() = default;

	EMapFormat version = EMapFormat::SOD;
	int32_t height = MAP_SIZE_MIDDLE;
	int32_t width = MAP_SIZE_MIDDLE;
	bool twoLevel = false;

	std::vector<PlayerInfo> players;
};

/// The map contains the map header, the tiles of the terrain, objects, heroes, towns, rumors...
class CMap: public CMapHeader
{
public:
	CMap() = default;
	~CMap();
	void initTerrain();

	TerrainTile& getTile(const int3 &tile);
	const TerrainTile& getTile(const int3 &tile) const;

	bool isInTheMap(const int3 &pos) const;

	///Use only this method when creating new map object instances
	void addNewObject(CGObjectInstance *obj);

	std::vector<bool> allowedArtifact;

	//Central lists of items in game. Position of item in the vectors below is their (instance) id.
	std::vector<CGObjectInstance*> objects;

private:
	/// a 3-dimensional array of terrain tiles, access is as follows: x, y, level. where level=1 is underground
	TerrainTile ***terrain = nullptr;
};
