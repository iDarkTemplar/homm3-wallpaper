/*
 * CMapDefines.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include <stdint.h>

#include "GameConstants.h"

/// The terrain tile describes the terrain type and the visual representation of the terrain.
/// Furthermore the struct defines whether the tile is visitable or/and blocked and which objects reside in it.
struct TerrainTile
{
	TerrainTile() = default;

	ETerrainType terType = ETerrainType::BORDER;
	uint8_t terView = 0;
	ERiverType riverType = ERiverType::NO_RIVER;
	uint8_t riverDir = 0;
	ERoadType roadType = ERoadType::NO_ROAD;
	uint8_t roadDir = 0;
	/// first two bits - how to rotate terrain graphic (next two - river graphic, next two - road);
	///	7th bit - whether tile is coastal (allows disembarking if land or block movement if water); 8th bit - Favorable Winds effect
	uint8_t extTileFlags = 0;
};
