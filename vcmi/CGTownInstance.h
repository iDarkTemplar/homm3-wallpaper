/*
 * CGTownInstance.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "CObjectHandler.h"

class CGDwelling;

class CSpecObjInfo
{
public:
	CSpecObjInfo() = default;
	virtual ~CSpecObjInfo() = default;

	const CGDwelling *owner = nullptr;
};

class CCreGenAsCastleInfo: public virtual CSpecObjInfo
{
public:
	CCreGenAsCastleInfo() = default;

	bool asCastle = false;
	uint32_t identifier = 0; // h3m internal identifier

	std::vector<bool> allowedFactions;
};

class CCreGenLeveledInfo: public virtual CSpecObjInfo
{
public:
	CCreGenLeveledInfo() = default;

	uint8_t minLevel = 1, maxLevel = 7; // minimal and maximal level of creature in dwelling: <1, 7>
};

class CCreGenLeveledCastleInfo: public CCreGenAsCastleInfo, public CCreGenLeveledInfo
{
public:
	CCreGenLeveledCastleInfo() = default;
};

class CGDwelling: public CGObjectInstance
{
public:
	std::unique_ptr<CSpecObjInfo> info; //random dwelling options; not serialized

	CGDwelling() = default;
	virtual ~CGDwelling() = default;
};

class CGTownInstance: public CGDwelling
{
public:
	CGTownInstance() = default;
	virtual ~CGTownInstance() = default;

	enum class TownType
	{
		BASIC = 0,
		HAS_FORT,
		HAS_CAPITOL,
	};

	uint32_t identifier = std::numeric_limits<decltype(identifier)>::max();
	int32_t alignment = 0xff;
	TownType town_type = TownType::HAS_FORT;
	std::optional<uint8_t> hero_type;
};
