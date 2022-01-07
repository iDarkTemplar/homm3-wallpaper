/*
 * CQuest.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

enum class CQuestMission
{
	NONE = 0,
	LEVEL = 1,
	PRIMARY_STAT = 2,
	KILL_HERO = 3,
	KILL_CREATURE = 4,
	ART = 5,
	ARMY = 6,
	RESOURCES = 7,
	HERO = 8,
	PLAYER = 9,
	KEYMASTER = 10
};

enum class CGSeerHutRewardType
{
	NOTHING,
	EXPERIENCE,
	MANA_POINTS,
	MORALE_BONUS,
	LUCK_BONUS,
	RESOURCES,
	PRIMARY_SKILL,
	SECONDARY_SKILL,
	ARTIFACT,
	SPELL,
	CREATURE
};
