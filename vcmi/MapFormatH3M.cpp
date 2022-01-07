/*
 * MapFormatH3M.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

#include "MapFormatH3M.h"
#include "CMap.h"
#include "CGTownInstance.h"

#include "CInputStream.h"

#include "data_maps.h"
#include "random.h"

enum class EVictoryConditionType
{
	ARTIFACT,
	GATHERTROOP,
	GATHERRESOURCE,
	BUILDCITY,
	BUILDGRAIL,
	BEATHERO,
	CAPTURECITY,
	BEATMONSTER,
	TAKEDWELLINGS,
	TAKEMINES,
	TRANSPORTITEM,
	WINSTANDARD = 255
};

enum class ELossConditionType
{
	LOSSCASTLE,
	LOSSHERO,
	TIMEEXPIRES,
	LOSSSTANDARD = 255
};

CMapLoaderH3M::CMapLoaderH3M(CInputStream *stream)
	: map(nullptr)
	, reader(stream)
	, inputStream(stream)
{
}

CMapLoaderH3M::~CMapLoaderH3M()
{
}

std::unique_ptr<CMap> CMapLoaderH3M::loadMap()
{
	// Init map object by parsing the input buffer
	map = new CMap();
	mapHeader = std::unique_ptr<CMapHeader>(dynamic_cast<CMapHeader*>(map));
	init();

	return std::unique_ptr<CMap>(dynamic_cast<CMap*>(mapHeader.release()));
}

void CMapLoaderH3M::init()
{
	inputStream->seek(0);

	readHeader();

	readDisposedHeroes();
	readAllowedArtifacts();
	readAllowedSpellsAbilities();
	readRumors();
	readPredefinedHeroes();
	readTerrain();
	readDefInfo();
	readObjects();
	readEvents();
}

void CMapLoaderH3M::readHeader()
{
	// Map version
	mapHeader->version = static_cast<EMapFormat>(reader.readUInt32());
	if ((mapHeader->version != EMapFormat::ROE)
		&& (mapHeader->version != EMapFormat::AB)
		&& (mapHeader->version != EMapFormat::SOD))
	{
		throw std::runtime_error("Invalid map format!");
	}

	// Read map name, description, dimensions,...
	reader.skip(1); /* unused: are any players on map */
	mapHeader->height = mapHeader->width = reader.readUInt32();
	mapHeader->twoLevel = reader.readBool();
	reader.skipString(); /* map name */
	reader.skipString(); /* map description */
	reader.skip(1); /* map difficulty */

	if (mapHeader->version != EMapFormat::ROE)
	{
		reader.skip(1); /* level limit */
	}

	readPlayerInfo();
	readVictoryLossConditions();
	readTeamInfo();
	readAllowedHeroes();
}

void CMapLoaderH3M::readPlayerInfo()
{
	for (size_t i = 0; i < mapHeader->players.size(); ++i)
	{
		bool canHumanPlay = reader.readBool();
		bool canComputerPlay = reader.readBool();

		mapHeader->players[i].isFactionActive = (canHumanPlay || canComputerPlay);

		// If nobody can play with this player
		if ((!(canHumanPlay || canComputerPlay)))
		{
			uint16_t totalFactions;

			if (mapHeader->version != EMapFormat::ROE)
			{
				totalFactions = GameConstants::F_NUMBER;
			}
			else
			{
				totalFactions = GameConstants::F_NUMBER - 1; // exclude conflux for ROE
			}

			if (mapHeader->players[i].allowedFactions.empty())
			{
				for (int fact = 0; fact < totalFactions; ++fact)
				{
					mapHeader->players[i].allowedFactions.push_back(static_cast<ETownType>(fact));
				}
			}

			switch (mapHeader->version)
			{
			case EMapFormat::SOD:
				reader.skip(13);
				break;
			case EMapFormat::AB:
				reader.skip(12);
				break;
			case EMapFormat::ROE:
				reader.skip(6);
				break;
			}

			continue;
		}

		reader.skip(1); /* ai tactic */

		if (mapHeader->version == EMapFormat::SOD)
		{
			reader.skip(1); /* something named p7 */
		}

		// Factions this player can choose
		uint16_t allowedFactions;
		// How many factions will be read from map
		uint16_t totalFactions;

		if (mapHeader->version != EMapFormat::ROE)
		{
			allowedFactions = reader.readUInt16();
			totalFactions = GameConstants::F_NUMBER;
		}
		else
		{
			allowedFactions = reader.readUInt8();
			totalFactions = GameConstants::F_NUMBER - 1; // exclude conflux for ROE
		}

		for (int fact = 0; fact < totalFactions; ++fact)
		{
			if (!(allowedFactions & (1 << fact)))
			{
				mapHeader->players[i].allowedFactions.push_back(static_cast<ETownType>(fact));
			}
		}

		if (mapHeader->players[i].allowedFactions.empty())
		{
			for (int fact = 0; fact < totalFactions; ++fact)
			{
				mapHeader->players[i].allowedFactions.push_back(static_cast<ETownType>(fact));
			}
		}

		mapHeader->players[i].isFactionRandom = reader.readBool();
		mapHeader->players[i].hasMainTown = reader.readBool();

		if (mapHeader->players[i].hasMainTown)
		{
			if (mapHeader->version != EMapFormat::ROE)
			{
				mapHeader->players[i].generateHeroAtMainTown = reader.readBool();
				reader.skip(1); // generate hero
			}
			else
			{
				mapHeader->players[i].generateHeroAtMainTown = true;
			}

			mapHeader->players[i].posOfMainTown = readInt3();
		}

		reader.skip(1); // has random hero

		uint8_t mainCustomHeroId = reader.readUInt8();

		if (mainCustomHeroId != 0xff)
		{
			reader.skip(1); /* main custom hero portrait */
			reader.skipString(); /* main custom hero name */
		}

		if (mapHeader->version != EMapFormat::ROE)
		{
			reader.skip(1); // unknown byte
			int heroCount = reader.readUInt8();
			reader.skip(3);

			for (int pp = 0; pp < heroCount; ++pp)
			{
				reader.skip(1); /* hero id */
				reader.skipString(); /* hero name */
			}
		}
	}
}

void CMapLoaderH3M::readVictoryLossConditions()
{
	auto vicCondition = static_cast<EVictoryConditionType>(reader.readUInt8());

	// Specific victory conditions
	if (vicCondition != EVictoryConditionType::WINSTANDARD)
	{
		reader.skip(1);
		reader.skip(1);

		switch (vicCondition)
		{
		case EVictoryConditionType::ARTIFACT:
			{
				reader.skip(1);
				if (mapHeader->version != EMapFormat::ROE)
					reader.skip(1);
				break;
			}

		case EVictoryConditionType::GATHERTROOP:
			{
				reader.skip(1);
				if (mapHeader->version != EMapFormat::ROE)
					reader.skip(1);
				reader.skip(4);
				break;
			}

		case EVictoryConditionType::GATHERRESOURCE:
			{
				reader.skip(1); /* object type */
				reader.skip(4); /* value */
				break;
			}

		case EVictoryConditionType::BUILDCITY:
			{
				skipInt3(); /* position */
				reader.skip(1); /* object type */
				reader.skip(1); /* object type */
				break;
			}
		case EVictoryConditionType::BUILDGRAIL:
			{
				skipInt3(); /* position */
				break;
			}
		case EVictoryConditionType::BEATHERO:
			{
				skipInt3(); /* position */
				break;
			}
		case EVictoryConditionType::CAPTURECITY:
			{
				skipInt3(); /* position */
				break;
			}
		case EVictoryConditionType::BEATMONSTER:
			{
				skipInt3(); /* position */
				break;
			}
		case EVictoryConditionType::TAKEDWELLINGS:
			{
				break;
			}
		case EVictoryConditionType::TAKEMINES:
			{
				break;
			}
		case EVictoryConditionType::TRANSPORTITEM:
			{
				reader.skip(1); /* object type */
				skipInt3(); /* position */
				break;
			}
		}
	}

	// Read loss conditions
	auto lossCond = static_cast<ELossConditionType>(reader.readUInt8());
	if (lossCond != ELossConditionType::LOSSSTANDARD)
	{
		switch (lossCond)
		{
		case ELossConditionType::LOSSCASTLE:
			{
				skipInt3(); /* position */
				break;
			}
		case ELossConditionType::LOSSHERO:
			{
				skipInt3(); /* position */
				break;
			}
		case ELossConditionType::TIMEEXPIRES:
			{
				reader.skip(2); /* value */
				break;
			}
		}
	}
}

void CMapLoaderH3M::readTeamInfo()
{
	uint8_t howManyTeams = reader.readUInt8();
	if (howManyTeams > 0)
	{
		// Teams
		reader.skip(static_cast<int>(PlayerColor::PLAYER_LIMIT_I)); /* player team number */
	}
}

void CMapLoaderH3M::readAllowedHeroes()
{
	const int bytes = mapHeader->version == EMapFormat::ROE ? 16 : 20;

	reader.skip(bytes); /* allowed heroes */

	// Probably reserved for further heroes
	if (mapHeader->version > EMapFormat::ROE)
	{
		int placeholdersQty = reader.readUInt32();

		reader.skip(placeholdersQty);
	}
}

void CMapLoaderH3M::readDisposedHeroes()
{
	// Reading disposed heroes (20 bytes)
	if (map->version >= EMapFormat::SOD)
	{
		uint8_t disp = reader.readUInt8();

		for (int g = 0; g < disp; ++g)
		{
			reader.skip(1); // hero id
			reader.skip(1); // hero portrait
			reader.skipString(); // hero name
			reader.skip(1); // players
		}
	}

	// omitting NULLS
	reader.skip(31);
}

void CMapLoaderH3M::readAllowedArtifacts()
{
	map->allowedArtifact.resize(GameConstants::ARTIFACTS_QUANTITY, true); // handle new artifacts, make them allowed by default

	// Reading allowed artifacts:  17 or 18 bytes
	if(map->version != EMapFormat::ROE)
	{
		const int bytes = map->version == EMapFormat::AB ? 17 : 18;

		readBitmask(map->allowedArtifact, bytes, GameConstants::ARTIFACTS_QUANTITY);
	}

	// ban combo artifacts
	if (map->version == EMapFormat::ROE || map->version == EMapFormat::AB)
	{
		for (const auto &artifact: combination_artifacts)
		{
			map->allowedArtifact[static_cast<size_t>(artifact)] = false;
		}

		if (map->version == EMapFormat::ROE)
		{
			map->allowedArtifact[static_cast<size_t>(ArtifactID::VIAL_OF_DRAGON_BLOOD)] = false;
			map->allowedArtifact[static_cast<size_t>(ArtifactID::ARMAGEDDONS_BLADE)] = false;
		}
	}
}

void CMapLoaderH3M::readAllowedSpellsAbilities()
{
	if (map->version >= EMapFormat::SOD)
	{
		// Reading allowed spells (9 bytes) and hero's abilities (4 bytes)
		reader.skip(13);
	}
}

void CMapLoaderH3M::readRumors()
{
	uint64_t rumNr = reader.readUInt32();

	for (uint64_t it = 0; it < rumNr; ++it)
	{
		reader.skipString(); /* rumor name */
		reader.skipString(); /* rumor text */
	}
}

void CMapLoaderH3M::readPredefinedHeroes()
{
	switch (map->version)
	{
	case EMapFormat::SOD:
		{
			// Disposed heroes
			for (int z = 0; z < GameConstants::HEROES_QUANTITY; ++z)
			{
				int custom =  reader.readUInt8();
				if (!custom)
				{
					continue;
				}

				bool hasExp = reader.readBool();
				if (hasExp)
				{
					reader.skip(4); /* hero exp */
				}

				bool hasSecSkills = reader.readBool();
				if (hasSecSkills)
				{
					int howMany = reader.readUInt32();
					reader.skip(2 * howMany);
				}

				skipArtifactsOfHero();

				bool hasCustomBio = reader.readBool();
				if (hasCustomBio)
				{
					reader.skipString(); /* biography */
				}

				reader.skip(1); // hero sex

				bool hasCustomSpells = reader.readBool();
				if (hasCustomSpells)
				{
					skipSpells();
				}

				bool hasCustomPrimSkills = reader.readBool();
				if (hasCustomPrimSkills)
				{
					reader.skip(GameConstants::PRIMARY_SKILLS); /* primary skills */
				}
			}
			break;
		}

	case EMapFormat::ROE:
		break;
	}
}

void CMapLoaderH3M::skipArtifactsOfHero()
{
	bool artSet = reader.readBool();

	// True if artifact set is not default (hero has some artifacts)
	if (artSet)
	{
		skipArtifact(16);

		// misc5 art //17
		if (map->version >= EMapFormat::SOD)
		{
			skipArtifact(1);
		}

		skipArtifact(1);

		// 19 // ???what is that? gap in file or what? - it's probably fifth slot..
		if (map->version > EMapFormat::ROE)
		{
			skipArtifact(1);
		}
		else
		{
			reader.skip(1);
		}

		// bag artifacts //20
		// number of artifacts in hero's bag
		int amount = reader.readUInt16();
		skipArtifact(amount);
	}
}

void CMapLoaderH3M::skipArtifact(size_t count)
{
	if (map->version == EMapFormat::ROE)
	{
		reader.skip(1 * count);
	}
	else
	{
		reader.skip(2 * count);
	}
}

void CMapLoaderH3M::readTerrain()
{
	map->initTerrain();

	// Read terrain
	for (int a = 0; a < 2; ++a)
	{
		if ((a == 1) && (!map->twoLevel))
		{
			break;
		}

		for (int c = 0; c < map->width; ++c)
		{
			for (int z = 0; z < map->height; ++z)
			{
				auto &tile = map->getTile(int3(z, c, a));
				tile.terType = ETerrainType(reader.readUInt8());
				tile.terView = reader.readUInt8();
				tile.riverType = static_cast<ERiverType>(reader.readUInt8());
				tile.riverDir = reader.readUInt8();
				tile.roadType = static_cast<ERoadType>(reader.readUInt8());
				tile.roadDir = reader.readUInt8();
				tile.extTileFlags = reader.readUInt8();
			}
		}
	}
}

void CMapLoaderH3M::readDefInfo()
{
	int defAmount = reader.readUInt32();

	templates.reserve(defAmount);

	// Read custom defs
	for (int idd = 0; idd < defAmount; ++idd)
	{
		ObjectTemplate tmpl;
		tmpl.readMap(reader);
		templates.push_back(tmpl);
	}
}

void CMapLoaderH3M::readObjects()
{
	int howManyObjs = reader.readUInt32();

	for(int ww = 0; ww < howManyObjs; ++ww)
	{
		CGObjectInstance *nobj = nullptr;

		int3 objPos = readInt3();

		int defnum = reader.readUInt32();

		ObjectTemplate &objTempl = templates.at(defnum);
		reader.skip(5);

		switch (objTempl.id)
		{
		case Obj::EVENT:
			{
				nobj = new CGObjectInstance();

				skipMessageAndGuards();

				reader.skip(4); // gained exp
				reader.skip(4); // mana diff
				reader.skip(1); // morale diff
				reader.skip(1); // luck diff

				skipResources();

				reader.skip(4); // primary skills

				int gabn = reader.readUInt8(); // Number of gained abilities
				reader.skip(gabn * 2);

				int gart = reader.readUInt8(); // Number of gained artifacts
				reader.skip(((map->version == EMapFormat::ROE) ? 1 : 2) * gart);

				int gspel = reader.readUInt8(); // Number of gained spells
				reader.skip(gspel);

				int gcre = reader.readUInt8(); //number of gained creatures
				skipCreatureSet(gcre);

				reader.skip(8);
				reader.skip(1); // available for
				reader.skip(1); // computer activate
				reader.skip(1); // remove after visit

				reader.skip(4);
				break;
			}

		case Obj::HERO:
		case Obj::RANDOM_HERO:
		case Obj::PRISON:
			{
				nobj = readHero();
				break;
			}

		case Obj::MONSTER:  //Monster
		case Obj::RANDOM_MONSTER:
		case Obj::RANDOM_MONSTER_L1:
		case Obj::RANDOM_MONSTER_L2:
		case Obj::RANDOM_MONSTER_L3:
		case Obj::RANDOM_MONSTER_L4:
		case Obj::RANDOM_MONSTER_L5:
		case Obj::RANDOM_MONSTER_L6:
		case Obj::RANDOM_MONSTER_L7:
			{
				nobj = new CGObjectInstance();

				if (map->version > EMapFormat::ROE)
				{
					reader.skip(4); // monster identifier
				}

				reader.skip(2); // creatures count
				reader.skip(1); // monster character

				bool hasMessage = reader.readBool();
				if (hasMessage)
				{
					reader.skipString(); // monster message
					skipResources();

					if (map->version == EMapFormat::ROE)
					{
						reader.skip(1);
					}
					else
					{
						reader.skip(2);
					}
				}

				reader.skip(1); // monster never flees
				reader.skip(1); // monster not growing
				reader.skip(2);
				break;
			}

		case Obj::OCEAN_BOTTLE:
		case Obj::SIGN:
			{
				nobj = new CGObjectInstance();
				reader.skipString(); // bottle message
				reader.skip(4);
				break;
			}

		case Obj::SEER_HUT:
			{
				nobj = new CGObjectInstance();
				skipSeerHut();
				break;
			}

		case Obj::WITCH_HUT:
			{
				nobj = new CGObjectInstance();

				// in RoE we cannot specify it - all are allowed (I hope)
				if (map->version > EMapFormat::ROE)
				{
					reader.skip(4); // allowed skills
				}
				break;
			}

		case Obj::SCHOLAR:
			{
				nobj = new CGObjectInstance();
				reader.skip(2); // bonus type and id
				reader.skip(6);
				break;
			}

		case Obj::GARRISON:
		case Obj::GARRISON2:
			{
				nobj = new CGObjectInstance();
				nobj->tempOwner = PlayerColor(reader.readUInt8());
				reader.skip(3);
				skipCreatureSet(7);
				if (map->version > EMapFormat::ROE)
				{
					reader.skip(1); // removable units
				}
				reader.skip(8);
				break;
			}

		case Obj::ARTIFACT:
		case Obj::RANDOM_ART:
		case Obj::RANDOM_TREASURE_ART:
		case Obj::RANDOM_MINOR_ART:
		case Obj::RANDOM_MAJOR_ART:
		case Obj::RANDOM_RELIC_ART:
			{
				nobj = new CGObjectInstance();
				skipMessageAndGuards();
				break;
			}

		case Obj::SPELL_SCROLL:
			{
				nobj = new CGObjectInstance();
				skipMessageAndGuards();
				reader.skip(4); // spell id
				break;
			}

		case Obj::RANDOM_RESOURCE:
		case Obj::RESOURCE:
			{
				nobj = new CGObjectInstance();
				skipMessageAndGuards();;
				reader.skip(4); // amount
				reader.skip(4);
				break;
			}

		case Obj::RANDOM_TOWN:
		case Obj::TOWN:
			{
				auto tobj  = readTown();
				nobj = tobj;
				break;
			}

		case Obj::MINE:
		case Obj::ABANDONED_MINE:
		case Obj::CREATURE_GENERATOR1:
		case Obj::CREATURE_GENERATOR2:
		case Obj::CREATURE_GENERATOR3:
		case Obj::CREATURE_GENERATOR4:
			{
				nobj = new CGObjectInstance();
				nobj->tempOwner = PlayerColor(reader.readUInt8());
				reader.skip(3);
				break;
			}

		case Obj::SHRINE_OF_MAGIC_INCANTATION:
		case Obj::SHRINE_OF_MAGIC_GESTURE:
		case Obj::SHRINE_OF_MAGIC_THOUGHT:
			{
				nobj = new CGObjectInstance();
				reader.skip(1); // spell id
				reader.skip(3);
				break;
			}

		case Obj::PANDORAS_BOX:
			{
				nobj = new CGObjectInstance();

				skipMessageAndGuards();

				reader.skip(4); // gained exp
				reader.skip(4); // mana
				reader.skip(1); // morale
				reader.skip(1); // luck

				skipResources();

				reader.skip(GameConstants::PRIMARY_SKILLS); // primary skills

				int gabn = reader.readUInt8(); // number of gained abilities
				reader.skip(gabn * 2); // gained abilities

				int gart = reader.readUInt8(); // number of gained artifacts
				if (map->version > EMapFormat::ROE)
				{
					reader.skip(gart * 2); // gained artifacts
				}
				else
				{
					reader.skip(gart); // gained artifacts
				}

				int gspel = reader.readUInt8(); // number of gained spells
				reader.skip(gspel); // gained spells

				int gcre = reader.readUInt8(); //number of gained creatures
				skipCreatureSet(gcre);

				reader.skip(8);
				break;
			}

		case Obj::GRAIL:
			{
				reader.skip(4); // grail radius
				continue;
			}

		case Obj::RANDOM_DWELLING: //same as castle + level range
		case Obj::RANDOM_DWELLING_LVL: //same as castle, fixed level
		case Obj::RANDOM_DWELLING_FACTION: //level range, fixed faction
			{
				auto dwelling = new CGDwelling();
				nobj = dwelling;
				CSpecObjInfo *spec = nullptr;

				switch (objTempl.id)
				{
				case Obj::RANDOM_DWELLING:
					spec = new CCreGenLeveledCastleInfo();
					break;
				case Obj::RANDOM_DWELLING_LVL:
					spec = new CCreGenAsCastleInfo();
					break;
				case Obj::RANDOM_DWELLING_FACTION:
					spec = new CCreGenLeveledInfo();
					break;
				default:
					throw std::runtime_error("Invalid random dwelling format");
				}

				spec->owner = dwelling;

				nobj->tempOwner = PlayerColor(reader.readUInt32());

				//216 and 217
				if (auto castleSpec = dynamic_cast<CCreGenAsCastleInfo *>(spec))
				{
					castleSpec->identifier = reader.readUInt32();
					if (!castleSpec->identifier)
					{
						castleSpec->asCastle = false;
						const int MASK_SIZE = 8;
						uint8_t mask[2];
						mask[0] = reader.readUInt8();
						mask[1] = reader.readUInt8();

						castleSpec->allowedFactions.clear();
						castleSpec->allowedFactions.resize(GameConstants::F_NUMBER, false);

						for (int i = 0; i < MASK_SIZE; ++i)
							castleSpec->allowedFactions[i] = ((mask[0] & (1 << i)) > 0);

						for (int i = 0; i < (GameConstants::F_NUMBER - MASK_SIZE); ++i)
							castleSpec->allowedFactions[i+MASK_SIZE] = ((mask[1] & (1 << i)) > 0);

						bool anyAllowed = false;

						for (int i = 0; i < GameConstants::F_NUMBER; ++i)
						{
							if (castleSpec->allowedFactions[i])
							{
								anyAllowed = true;
								break;
							}
						}

						if (!anyAllowed)
						{
							for (int i = 0; i < GameConstants::F_NUMBER; ++i)
							{
								castleSpec->allowedFactions[i] = true;
							}
						}
					}
					else
					{
						castleSpec->asCastle = true;
					}
				}

				//216 and 218
				if (auto lvlSpec = dynamic_cast<CCreGenLeveledInfo *>(spec))
				{
					lvlSpec->minLevel = std::min(std::max(reader.readUInt8(), uint8_t(1)), uint8_t(7));
					lvlSpec->maxLevel = std::min(std::max(reader.readUInt8(), uint8_t(1)), uint8_t(7));

					if (lvlSpec->minLevel > lvlSpec->maxLevel)
					{
						lvlSpec->minLevel = 1;
						lvlSpec->maxLevel = 7;
					}
				}

				dwelling->info.reset(spec);
				break;
			}

		case Obj::QUEST_GUARD:
			{
				nobj = new CGObjectInstance();
				skipQuest();
				break;
			}

		case Obj::HERO_PLACEHOLDER: //hero placeholder
			{
				nobj = new CGObjectInstance();

				nobj->tempOwner = PlayerColor(reader.readUInt8());

				int htid = reader.readUInt8(); // hero type id
				nobj->subID = htid;

				if (htid == 0xff)
				{
					reader.skip(1); // hero power
				}
				break;
			}

		case Obj::SHIPYARD:
		case Obj::LIGHTHOUSE:
			{
				nobj = new CGObjectInstance();
				nobj->tempOwner = PlayerColor(reader.readUInt32());
				break;
			}

		default: //any other object
			{
				nobj = new CGObjectInstance();
				break;
			}
		}

		nobj->pos = objPos;
		nobj->ID = objTempl.id;

		if (nobj->ID != Obj::HERO && nobj->ID != Obj::HERO_PLACEHOLDER && nobj->ID != Obj::PRISON)
		{
			nobj->subID = objTempl.subid;
		}

		nobj->appearance = objTempl;
		map->addNewObject(nobj);
	}
}

void CMapLoaderH3M::skipCreatureSet(int number)
{
	const bool version = (map->version > EMapFormat::ROE);

	reader.skip(((version ? 2 : 1) + 2) * number);
}

CGObjectInstance* CMapLoaderH3M::readHero()
{
	auto nhi = new CGObjectInstance();

	if (map->version > EMapFormat::ROE)
	{
		reader.skip(4); // quest identifier
	}

	nhi->tempOwner = PlayerColor(reader.readUInt8());
	nhi->subID = reader.readUInt8();

	bool hasName = reader.readBool();
	if (hasName)
	{
		reader.skipString(); // skip name
	}

	if (map->version > EMapFormat::AB)
	{
		bool hasExp = reader.readBool();
		if (hasExp)
		{
			reader.skip(4); // skip experience
		}
	}
	else
	{
		reader.skip(4); // skip experience
	}

	bool hasPortrait = reader.readBool();
	if (hasPortrait)
	{
		reader.skip(1); // skip portrait
	}

	bool hasSecSkills = reader.readBool();
	if (hasSecSkills)
	{
		int howMany = reader.readUInt32();
		reader.skip(2 * howMany); // skip secondary skills
	}

	bool hasGarrison = reader.readBool();
	if (hasGarrison)
	{
		skipCreatureSet(7);
	}

	reader.skip(1); // formation
	skipArtifactsOfHero();
	reader.skip(1); // patrol radius

	if (map->version > EMapFormat::ROE)
	{
		bool hasCustomBiography = reader.readBool();
		if (hasCustomBiography)
		{
			reader.skipString(); // skip custom biography
		}

		reader.skip(1); // hero sex
	}

	// Spells
	if (map->version > EMapFormat::AB)
	{
		bool hasCustomSpells = reader.readBool();
		if (hasCustomSpells)
		{
			skipSpells();
		}
	}
	else if (map->version == EMapFormat::AB)
	{
		// we can read one spell
		reader.skip(1);
	}

	if (map->version > EMapFormat::AB)
	{
		bool hasCustomPrimSkills = reader.readBool();
		if (hasCustomPrimSkills)
		{
			reader.skip(GameConstants::PRIMARY_SKILLS); // skip primary skills
		}
	}

	reader.skip(16);
	return nhi;
}

void CMapLoaderH3M::skipSeerHut()
{
	CQuestMission missionType = CQuestMission::NONE;

	if (map->version > EMapFormat::ROE)
	{
		missionType = skipQuest();
	}
	else
	{
		//RoE
		int artID = reader.readUInt8();
		if (artID != 255)
		{
			// not none quest
			missionType = CQuestMission::ART;
		}
		else
		{
			missionType = CQuestMission::NONE;
		}
	}

	if (missionType != CQuestMission::NONE)
	{
		auto rewardType = static_cast<CGSeerHutRewardType>(reader.readUInt8());

		switch (rewardType)
		{
		case CGSeerHutRewardType::EXPERIENCE:
			{
				reader.skip(4);
				break;
			}

		case CGSeerHutRewardType::MANA_POINTS:
			{
				reader.skip(4);
				break;
			}
		case CGSeerHutRewardType::MORALE_BONUS:
			{
				reader.skip(1);
				break;
			}
		case CGSeerHutRewardType::LUCK_BONUS:
			{
				reader.skip(1);
				break;
			}
		case CGSeerHutRewardType::RESOURCES:
			{
				reader.skip(5); // resource id and count
				break;
			}
		case CGSeerHutRewardType::PRIMARY_SKILL:
			{
				reader.skip(2); // primary skill and amount
				break;
			}
		case CGSeerHutRewardType::SECONDARY_SKILL:
			{
				reader.skip(2); // secondary skill and amount
				break;
			}
		case CGSeerHutRewardType::ARTIFACT:
			{
				if (map->version == EMapFormat::ROE)
				{
					reader.skip(1);
				}
				else
				{
					reader.skip(2);
				}
				break;
			}
		case CGSeerHutRewardType::SPELL:
			{
				reader.skip(1);
				break;
			}
		case CGSeerHutRewardType::CREATURE:
			{
				if (map->version > EMapFormat::ROE)
				{
					reader.skip(4); // creature id and amount
				}
				else
				{
					reader.skip(3); // creature id and amount
				}
				break;
			}
		}

		reader.skip(2);
	}
	else
	{
		// missionType == 255
		reader.skip(3);
	}
}

CQuestMission CMapLoaderH3M::skipQuest()
{
	auto missionType = static_cast<CQuestMission>(reader.readUInt8());

	switch (missionType)
	{
	case CQuestMission::NONE:
		return CQuestMission::NONE;

	case CQuestMission::PRIMARY_STAT:
	case CQuestMission::LEVEL:
	case CQuestMission::KILL_HERO:
	case CQuestMission::KILL_CREATURE:
		reader.skip(4);
		break;

	case CQuestMission::ART:
		{
			int artNumber = reader.readUInt8();
			for (int yy = 0; yy < artNumber; ++yy)
			{
				int artid = reader.readUInt16();
				map->allowedArtifact[artid] = false; //these are unavailable for random generation
			}
			break;
		}

	case CQuestMission::ARMY:
		{
			int typeNumber = reader.readUInt8();
			reader.skip(typeNumber * 4);
			break;
		}

	case CQuestMission::RESOURCES:
		reader.skip(7 * 4);
		break;

	case CQuestMission::HERO:
	case CQuestMission::PLAYER:
		reader.skip(1);
		break;
	}

	reader.skip(4); // quest days limit
	reader.skipString(); // first visit text
	reader.skipString(); // next visit text
	reader.skipString(); // completed text

	return missionType;
}

CGTownInstance* CMapLoaderH3M::readTown()
{
	auto nt = new CGTownInstance();
	if (map->version > EMapFormat::ROE)
	{
		nt->identifier = reader.readUInt32(); // identifier
	}

	nt->tempOwner = PlayerColor(reader.readUInt8());
	bool hasName = reader.readBool();
	if (hasName)
	{
		reader.skipString(); // name
	}

	bool hasGarrison = reader.readBool();
	if (hasGarrison)
	{
		skipCreatureSet(7);
	}

	reader.skip(1); // formation

	bool hasCustomBuildings = reader.readBool();
	if (hasCustomBuildings)
	{
		std::vector<bool> builtBuildings;
		builtBuildings.resize(48, true);

		readBitmask(builtBuildings, 6, 48, false);

		if (builtBuildings[static_cast<size_t>(BuildingID::CAPITOL)])
		{
			nt->town_type = CGTownInstance::TownType::HAS_CAPITOL;
		}
		else if (builtBuildings[static_cast<size_t>(BuildingID::FORT)])
		{
			nt->town_type = CGTownInstance::TownType::HAS_FORT;
		}
		else
		{
			nt->town_type = CGTownInstance::TownType::BASIC;
		}

		reader.skip(6); // forbidden buildings
	}
	// Standard buildings
	else
	{
		bool has_fort = reader.readBool();

		nt->town_type = has_fort ? CGTownInstance::TownType::HAS_FORT : CGTownInstance::TownType::BASIC;
	}

	if (map->version > EMapFormat::ROE)
	{
		reader.skip(9); // obligatory spells
	}

	reader.skip(9); // possible spells

	// Read castle events
	int numberOfEvent = reader.readUInt32();

	for (int gh = 0; gh < numberOfEvent; ++gh)
	{
		reader.skipString(); // name
		reader.skipString(); // message

		skipResources();

		reader.skip(1); // players
		if (map->version > EMapFormat::AB)
		{
			reader.skip(1); // human affected
		}

		reader.skip(1); // computer affected
		reader.skip(2); // first occurence
		reader.skip(1); // next occurence
		reader.skip(17);
		reader.skip(9); // new buildings
		reader.skip(7*2); // new creatures
		reader.skip(4);
	}

	if (map->version > EMapFormat::AB)
	{
		// 0xFF = any, other = player number
		nt->alignment = reader.readUInt8();
	}

	reader.skip(3);

	return nt;
}

void CMapLoaderH3M::readEvents()
{
	int numberOfEvents = reader.readUInt32();
	for (int yyoo = 0; yyoo < numberOfEvents; ++yyoo)
	{
		reader.skipString(); // name
		reader.skipString(); // message

		skipResources();
		reader.skip(1); // players
		if (map->version > EMapFormat::AB)
		{
			reader.skip(1); // human affected
		}

		reader.skip(1); // computer affected
		reader.skip(2); // first occurence
		reader.skip(1); // next occurence

		reader.skip(17);
	}
}

void CMapLoaderH3M::skipMessageAndGuards()
{
	bool hasMessage = reader.readBool();
	if (hasMessage)
	{
		reader.skipString();
		bool hasGuards = reader.readBool();
		if (hasGuards)
		{
			skipCreatureSet(7);
		}

		reader.skip(4);
	}
}

void CMapLoaderH3M::skipSpells()
{
	reader.skip(9);
}

void CMapLoaderH3M::skipResources()
{
	reader.skip(4 * 7);
}

void CMapLoaderH3M::readBitmask(std::vector<bool> &dest, const int byteCount, const int limit, bool negate)
{
	for (int byte = 0; byte < byteCount; ++byte)
	{
		const uint8_t mask = reader.readUInt8();
		for (int bit = 0; bit < 8; ++bit)
		{
			if (byte * 8 + bit < limit)
			{
				const bool flag = mask & (1 << bit);
				if ((negate && flag) || (!negate && !flag)) // FIXME: check PR388
				{
					dest[byte * 8 + bit] = false;
				}
			}
		}
	}
}

int3 CMapLoaderH3M::readInt3()
{
	int3 p;

	p.x = reader.readUInt8();
	p.y = reader.readUInt8();
	p.z = reader.readUInt8();

	return p;
}

void CMapLoaderH3M::skipInt3()
{
	reader.skip(3);
}
