/*
 * CObjectHandler.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "ObjectTemplate.h"

#include "int3.h"

class CGObjectInstance
{
public:
	/// Position of bottom-right corner of object on map
	int3 pos = int3(-1);

	/// Type of object, e.g. town, hero, creature.
	Obj ID = Obj::NO_OBJ;

	/// Subtype of object, depends on type
	int32_t subID = -1;

	/// Defines appearance of object on map (animation, blocked tiles, blit order, etc)
	ObjectTemplate appearance;

	/// Current owner of an object (when below PLAYER_LIMIT)
	PlayerColor tempOwner = PlayerColor::NEUTRAL;

	CGObjectInstance() = default;
	virtual ~CGObjectInstance() = default;
};
