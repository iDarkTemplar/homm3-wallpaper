/*
 * ObjectTemplate.h, part of VCMI engine
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
#include "int3.h"

class CBinaryReader;

class ObjectTemplate
{
public:
	/// H3 ID/subID of this object
	Obj id = Obj::NO_OBJ;
	int32_t subid = 0;

	/// print priority, objects with higher priority will be print first, below everything else
	int32_t printPriority = 0;

	/// animation file that should be used to display object
	std::string animationFile;

	bool isVisitable = false;

	ObjectTemplate() = default;

	void readMap(CBinaryReader &reader);
};
