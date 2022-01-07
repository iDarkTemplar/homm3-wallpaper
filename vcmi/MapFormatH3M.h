/*
 * MapFormatH3M.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "int3.h"

#include "CBinaryReader.h"
#include "CMap.h"
#include "CQuest.h"
#include "ObjectTemplate.h"

#include <list>
#include <map>
#include <memory>
#include <set>

class CGObjectInstance;
class CGDwelling;
class CGTownInstance;
class CInputStream;

class CMapLoaderH3M
{
public:
	/**
	 * Default constructor.
	 *
	 * @param stream a stream containing the map data
	 */
	explicit CMapLoaderH3M(CInputStream *stream);

	/**
	 * Destructor.
	 */
	~CMapLoaderH3M();

	/**
	 * Loads the VCMI/H3 map file.
	 *
	 * @return a unique ptr of the loaded map class
	 */
	std::unique_ptr<CMap> loadMap();

private:
	/**
	 * Initializes the map object from parsing the input buffer.
	 */
	void init();

	/**
	 * Reads the map header.
	 */
	void readHeader();

	/**
	 * Reads player information.
	 */
	void readPlayerInfo();

	/**
	 * Reads victory/loss conditions.
	 */
	void readVictoryLossConditions();

	/**
	 * Reads team information.
	 */
	void readTeamInfo();

	/**
	 * Reads the list of allowed heroes.
	 */
	void readAllowedHeroes();

	/**
	 * Reads the list of disposed heroes.
	 */
	void readDisposedHeroes();

	/**
	 * Reads the list of allowed artifacts.
	 */
	void readAllowedArtifacts();

	/**
	 * Reads the list of allowed spells and abilities.
	 */
	void readAllowedSpellsAbilities();

	/**
	 * Loads artifacts of a hero.
	 *
	 * @param hero the hero which should hold those artifacts
	 */
	void skipArtifactsOfHero();

	/**
	 * Loads an artifact.
	 *
	 */
	void skipArtifact(size_t count);

	/**
	 * Read rumors.
	 */
	void readRumors();

	/**
	 * Reads predefined heroes.
	 */
	void readPredefinedHeroes();

	/**
	 * Reads terrain data.
	 */
	void readTerrain();

	/**
	 * Reads custom(map) def information.
	 */
	void readDefInfo();

	/**
	 * Reads objects(towns, mines,...).
	 */
	void readObjects();

	/**
	 * Reads a creature set.
	 *
	 * @param number the count of creatures to read
	 */
	void skipCreatureSet(int number);

	/**
	 * Reads a hero.
	 *
	 * @param idToBeGiven the object id which should be set for the hero
	 * @return a object instance
	 */
	CGObjectInstance* readHero();

	/**
	 * Reads a seer hut.
	 *
	 * @return the initialized seer hut object
	 */
	void skipSeerHut();

	/**
	 * Reads a quest for the given quest guard.
	 *
	 * @param guard the quest guard where that quest should be applied to
	 */
	CQuestMission skipQuest();

	/**
	 * Reads a town.
	 *
	 * @param castleID the id of the castle type
	 * @return the loaded town object
	 */
	CGTownInstance* readTown();

	/**
	 * Converts buildings to the specified castle id.
	 *
	 * @param h3m the ids of the buildings
	 * @param castleID the castle id
	 * @param addAuxiliary true if the village hall should be added
	 * @return the converted buildings
	 */
	std::set<BuildingID> convertBuildings(const std::set<BuildingID> &h3m, int castleID, bool addAuxiliary = true);

	/**
	 * Reads events.
	 */
	void readEvents();

	/**
	 * read optional message and optional guards
	 */
	void skipMessageAndGuards();

	void skipSpells();

	void skipResources();

	/** Reads bitmask to boolean vector
	 * @param dest destination vector, shall be filed with "true" values
	 * @param byteCount size in bytes of bimask
	 * @param limit max count of vector elements to alter
	 * @param negate if true then set bit in mask means clear flag in vertor
	 */
	void readBitmask(std::vector<bool> &dest, const int byteCount, const int limit, bool negate = true);

	/**
	* Helper to read map position
	*/
	int3 readInt3();

	void skipInt3();

	/** List of templates loaded from the map, used on later stage to create
	 *  objects but not needed for fully functional CMap */
	std::vector<ObjectTemplate> templates;

	/** ptr to the map object which gets filled by data from the buffer */
	CMap* map;

	/**
	 * ptr to the map header object which gets filled by data from the buffer.
	 * (when loading a map then the mapHeader ptr points to the same object)
	 */
	std::unique_ptr<CMapHeader> mapHeader;

	CBinaryReader reader;
	CInputStream *inputStream;
};
