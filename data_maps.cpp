/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "data_maps.h"

const std::vector<ArtifactID> treasure_artifacts = {
	ArtifactID::AMULET_OF_THE_UNDERTAKER,
	ArtifactID::BADGE_OF_COURAGE,
	ArtifactID::BIRD_OF_PERCEPTION,
	ArtifactID::BOW_OF_ELVEN_CHERRYWOOD,
	ArtifactID::BREASTPLATE_OF_PETRIFIED_WOOD,
	ArtifactID::CAPE_OF_CONJURING,
	ArtifactID::CARDS_OF_PROPHECY,
	ArtifactID::CENTAUR_AXE,
	ArtifactID::CHARM_OF_MANA,
	ArtifactID::CLOVER_OF_FORTUNE,
	ArtifactID::COLLAR_OF_CONJURING,
	ArtifactID::CREST_OF_VALOR,
	ArtifactID::DRAGONBONE_GREAVES,
	ArtifactID::GLYPH_OF_GALLANTRY,
	ArtifactID::HELM_OF_THE_ALABASTER_UNICORN,
	ArtifactID::HOURGLASS_OF_THE_EVIL_HOUR,
	ArtifactID::LADYBIRD_OF_LUCK,
	ArtifactID::LEGS_OF_LEGION,
	ArtifactID::MYSTIC_ORB_OF_MANA,
	ArtifactID::NECKLACE_OF_SWIFTNESS,
	ArtifactID::PENDANT_OF_DEATH,
	ArtifactID::PENDANT_OF_DISPASSION,
	ArtifactID::PENDANT_OF_FREE_WILL,
	ArtifactID::PENDANT_OF_HOLINESS,
	ArtifactID::PENDANT_OF_LIFE,
	ArtifactID::PENDANT_OF_TOTAL_RECALL,
	ArtifactID::QUIET_EYE_OF_THE_DRAGON,
	ArtifactID::RING_OF_CONJURING,
	ArtifactID::RING_OF_VITALITY,
	ArtifactID::SHIELD_OF_THE_DWARVEN_LORDS,
	ArtifactID::SKULL_HELMET,
	ArtifactID::SPECULUM,
	ArtifactID::SPIRIT_OF_OPPRESSION,
	ArtifactID::SPYGLASS,
	ArtifactID::STILL_EYE_OF_THE_DRAGON,
	ArtifactID::STOIC_WATCHMAN,
	ArtifactID::TALISMAN_OF_MANA
};

const std::vector<ArtifactID> minor_artifacts = {
	ArtifactID::ARMOR_OF_WONDER,
	ArtifactID::BLACKSHARD_OF_THE_DEAD_KNIGHT,
	ArtifactID::BOOTS_OF_SPEED,
	ArtifactID::BOWSTRING_OF_THE_UNICORNS_MANE,
	ArtifactID::BUCKLER_OF_THE_GNOLL_KING,
	ArtifactID::CROWN_OF_THE_SUPREME_MAGI,
	ArtifactID::DRAGON_WING_TABARD,
	ArtifactID::EMBLEM_OF_COGNIZANCE,
	ArtifactID::EQUESTRIANS_GLOVES,
	ArtifactID::GREATER_GNOLLS_FLAIL,
	ArtifactID::HELM_OF_CHAOS,
	ArtifactID::INEXHAUSTIBLE_CART_OF_ORE,
	ArtifactID::INEXHAUSTIBLE_CART_OF_LUMBER,
	ArtifactID::LOINS_OF_LEGION,
	ArtifactID::RED_GRADON_FLAME_TONGUE,
	ArtifactID::RIB_CAGE,
	ArtifactID::RING_OF_LIFE,
	ArtifactID::SCALES_OF_THE_GREATER_BASILISK,
	ArtifactID::SHIELD_OF_THE_YAWNING_DEAD,
	ArtifactID::TORSO_OF_LEGION,
	ArtifactID::VAMPIRES_COWL
};

const std::vector<ArtifactID> major_artifacts = {
	ArtifactID::AMBASSADORS_SASH,
	ArtifactID::ANGEL_FEATHERS_ARROWS,
	ArtifactID::ARMS_OF_LEGION,
	ArtifactID::BREASTPLATE_OF_BRIMSTONE,
	ArtifactID::CAPE_OF_VELOCITY,
	ArtifactID::DEAD_MANS_BOOTS,
	ArtifactID::DIPLOMATS_RING,
	ArtifactID::DRAGON_SCALE_SHIELD,
	ArtifactID::ENDLESS_BAG_OF_GOLD,
	ArtifactID::ENDLESS_PURSE_OF_GOLD,
	ArtifactID::EVERFLOWING_CRYSTAL_CLOAK,
	ArtifactID::EVERPOURING_VIAL_OF_MERCURY,
	ArtifactID::EVERSMOKING_RING_OF_SULFUR,
	ArtifactID::GARNITURE_OF_INTERFERENCE,
	ArtifactID::GOLDEN_BOW,
	ArtifactID::HEAD_OF_LEGION,
	ArtifactID::HELLSTORM_HELMET,
	ArtifactID::NECKLACE_OF_DRAGONTEETH,
	ArtifactID::NECKLACE_OF_OCEAN_GUIDANCE,
	ArtifactID::OGRES_CLUB_OF_HAVOC,
	ArtifactID::ORB_OF_DRIVING_RAIN,
	ArtifactID::ORB_OF_THE_FIRMAMENT,
	ArtifactID::ORB_OF_SILT,
	ArtifactID::ORB_OF_TEMPESTUOUS_FIRE,
	ArtifactID::PENDANT_OF_COURAGE,
	ArtifactID::PENDANT_OF_NEGATIVITY,
	ArtifactID::PENDANT_OF_SECOND_SIGHT,
	ArtifactID::RECANTERS_CLOAK,
	ArtifactID::RING_OF_INFINITE_GEMS,
	ArtifactID::RING_OF_THE_WAYFARER,
	ArtifactID::SHACKLES_OF_WAR,
	ArtifactID::SHIELD_OF_THE_DAMNED,
	ArtifactID::SPHERE_OF_PERMANENCE,
	ArtifactID::STATESMANS_MEDAL,
	ArtifactID::SURCOAT_OF_COUNTERPOISE,
	ArtifactID::SWORD_OF_HELLFIRE,
	ArtifactID::TARG_OF_THE_RAMPAGING_OGRE,
	ArtifactID::TUNIC_OF_THE_CYCLOPS_KING,
	ArtifactID::VIAL_OF_LIFEBLOOD
};

const std::vector<ArtifactID> relic_artifacts = {
	ArtifactID::ANGEL_WINGS,
	ArtifactID::ARMAGEDDONS_BLADE,
	ArtifactID::BOOTS_OF_LEVITATION,
	ArtifactID::BOOTS_OF_POLARITY,
	ArtifactID::CELESTIAL_NECKLACE_OF_BLISS,
	ArtifactID::CROWN_OF_DRAGONTOOTH,
	ArtifactID::DRAGON_SCALE_ARMOR,
	ArtifactID::ENDLESS_SACK_OF_GOLD,
	ArtifactID::HELM_OF_HEAVENLY_ENLIGHTENMENT,
	ArtifactID::LIONS_SHIELD_OF_COURAGE,
	ArtifactID::ORB_OF_VULNERABILITY,
	ArtifactID::ORB_OF_INHIBITION,
	ArtifactID::SANDALS_OF_THE_SAINT,
	ArtifactID::SEA_CAPTAINS_HAT,
	ArtifactID::SENTINELS_SHIELD,
	ArtifactID::SPELLBINDERS_HAT,
	ArtifactID::SWORD_OF_JUDGEMENT,
	ArtifactID::THUNDER_HELMET,
	ArtifactID::TITANS_CUIRASS,
	ArtifactID::TITANS_GLADIUS,
	ArtifactID::TOME_OF_AIR_MAGIC,
	ArtifactID::TOME_OF_EARTH_MAGIC,
	ArtifactID::TOME_OF_FIRE_MAGIC,
	ArtifactID::TOME_OF_WATER_MAGIC,
	ArtifactID::VIAL_OF_DRAGON_BLOOD,
};

const std::vector<ArtifactID> combination_artifacts = {
	ArtifactID::ANGELIC_ALLIANCE,
	ArtifactID::CLOAK_OF_THE_UNDEAD_KING,
	ArtifactID::ELIXIR_OF_LIFE,
	ArtifactID::ARMOR_OF_THE_DAMNED,
	ArtifactID::STATUE_OF_LEGION,
	ArtifactID::POWER_OF_THE_DRAGON_FATHER,
	ArtifactID::TITANS_THUNDER,
	ArtifactID::ADMIRALS_HAT,
	ArtifactID::BOW_OF_THE_SHARPSHOOTER,
	ArtifactID::WIZARDS_WELL,
	ArtifactID::RING_OF_THE_MAGI,
	ArtifactID::CORNUCOPIA
};

const std::map<ETownType, std::vector<std::string> > dwellings_map =
{
	{
		ETownType::CASTLE,
		{
			"avgpike0.def",
			"avgcros0.def",
			"avggrff0.def",
			"avgswor0.def",
			"avgmonk0.def",
			"avgcavl0.def",
			"avgangl0.def",
		}
	},
	{
		ETownType::RAMPART,
		{
			"avgcent0.def",
			"avgdwrf0.def",
			"avgelf0.def",
			"avgpega0.def",
			"avgtree0.def",
			"avg2uni.def",
			"avggdrg0.def",
		}
	},
	{
		ETownType::TOWER,
		{
			"avggrem0.def",
			"avggarg0.def",
			"avggolm0.def",
			"avgmage0.def",
			"avggeni0.def",
			"avgnaga0.def",
			"avgtitn0.def",
		}
	},
	{
		ETownType::INFERNO,
		{
			"avgimp0.def",
			"avggogs0.def",
			"avghell0.def",
			"avgdemn0.def",
			"avgpit0.def",
			"avgefre0.def",
			"avgdevl0.def",
		}
	},
	{
		ETownType::NECROPOLIS,
		{
			"avgskel0.def",
			"avgzomb0.def",
			"avgwght0.def",
			"avgvamp0.def",
			"avglich0.def",
			"avgbkni0.def",
			"avgbone0.def",
		}
	},
	{
		ETownType::DUNGEON,
		{
			"avgtrog0.def",
			"avgharp0.def",
			"avgbhld0.def",
			"avgmdsa0.def",
			"avgmino0.def",
			"avgmant0.def",
			"avgrdrg0.def",
		}
	},
	{
		ETownType::STRONGHOLD,
		{
			"avggobl0.def",
			"avgwolf0.def",
			"avgorcg0.def",
			"avgogre0.def",
			"avgrocs0.def",
			"avgcycl0.def",
			"avgbhmt0.def",
		}
	},
	{
		ETownType::FORTRESS,
		{
			"avggnll0.def",
			"avglzrd0.def",
			"avgdfly0.def",
			"avgbasl0.def",
			"avggorg0.def",
			"avgwyvn0.def",
			"avghydr0.def",
		}
	},
	{
		ETownType::CONFLUX,
		{
			"avgpixie.def",
			"avg2ela.def",
			"avg2elw.def",
			"avg2elf.def",
			"avg2ele.def",
			"avgelp.def",
			"avgfbrd.def",
		}
	},
};

const std::vector<std::vector<std::string> > monsters_map =
{
	// level 1
	{
		"avwpike.def", // Pikeman
		"avwpikx0.def", // Halberdier
		"avwcent0.def", // Centaur
		"avwcenx0.def", // Centaur Captain
		"avwgrem0.def", // Gremlin
		"avwgrex0.def", // Master Gremlin
		"avwimp0.def", // Imp
		"avwimpx0.def", // Familiar
		"avwskel0.def", // Skeleton
		"avwskex0.def", // Skeleton Warrior
		"avwtrog0.def", // Troglodyte
		"avwinfr.def", // Infernal Troglodyte
		"avwgobl0.def", // Goblin
		"avwgobx0.def", // Hobgoblin
		"avwgnll0.def", // Gnoll
		"avwgnlx0.def", // Gnoll Marauder
		"avwpixie.def", // Pixie
		"avwsprit.def", // Sprite
		//
		"avwpeas.def", // Peasant
		"avwhalf.def", // Halfling
	},
	// level 2
	{
		"avwlcrs.def", // Archer
		"avwhcrs.def", // Marksman
		"avwdwrf0.def", // Dwarf
		"avwdwrx0.def", // Battle Dwarf
		"avwgarg0.def", // Stone Gargoyle
		"avwgarx0.def", // Obsidian Gargoyle
		"avwgog0.def", // Gog
		"avwgogx0.def", // Magog
		"avwzomb0.def", // Walking Dead
		"avwzomx0.def", // Zombie
		"avwharp0.def", // Harpy
		"avwharx0.def", // Harpy Hag
		"avwwolf0.def", // Wolf Rider
		"avwwolx0.def", // Wolf Raider
		"avwlizr.def", // Lizardman
		"avwlizx0.def", // Lizard Warrior
		"avwelma0.def", // Air Elemental
		"avwstorm.def", // Storm Elemental
		//
		"avwboar.def", // Boar
		"avwrog.def", // Rogue
	},
	// level 3
	{
		"avwgrif.def", // Griffin
		"avwgrix0.def", // Royal Griffin
		"avwelfw0.def", // Wood Elf
		"avwelfx0.def", // Grand Elf
		"avwgolm0.def", // Stone Golem
		"avwgolx0.def", // Iron Golem
		"avwhoun0.def", // Hell Hound
		"avwhoux0.def", // Cerberus
		"avwwigh.def", // Wight
		"avwwigx0.def", // Wraith
		"avwbehl0.def", // Beholder
		"avwbehx0.def", // Evil Eye
		"avworc0.def", // Orc
		"avworcx0.def", // Orc Chieftain
		"avwdfly.def", // Serpent Fly
		"avwdfir.def", // Dragon Fly
		"avwelmw0.def", // Water Elemental
		"avwicee.def", // Ice Elemental
		//
		"avwmumy.def", // Mummy
		"avwnomd.def", // Nomad
	},
	// level 4
	{
		"avwswrd0.def", // Swordsman
		"avwswrx0.def", // Crusader
		"avwpega0.def", // Pegasus
		"avwpegx0.def", // Silver Pegasus
		"avwmage0.def", // Mage Mage
		"avwmagx0.def", // Arch Mage
		"avwdemn0.def", // Demon
		"avwdemx0.def", // Horned Demon
		"avwvamp0.def", // Vampire
		"avwvamx0.def", // Vampire Lord
		"avwmeds.def", // Medusa
		"avwmedx0.def", // Medusa Queen
		"avwogre0.def", // Ogre
		"avwogrx0.def", // Ogre Mage
		"avwbasl.def", // Basilisk
		"avwgbas.def", // Greater Basilisk
		"avwelmf0.def", // Fire Elemental
		"avwnrg.def", // Energy Elemental
		//
		"avwsharp.def", // Sharpshooter
	},
	// level 5
	{
		"avwmonk.def", // Monk
		"avwmonx0.def", // Zealot
		"avwtree0.def", // Dendroid Guard
		"avwtrex0.def", // Dendroid Soldier
		"avwgeni0.def", // Genie
		"avwgenx0.def", // Master Genie
		"avwpitf0.def", // Pit Fiend
		"avwpitx0.def", // Pit Lord
		"avwlich0.def", // Lich
		"avwlicx0.def", // Power Lich
		"avwmino.def", // Minotaur
		"avwminx0.def", // Minotaur King
		"avwroc0.def", // Roc
		"avwrocx0.def", // Thunderbird
		"avwgorg.def", // Gorgon
		"avwgorx0.def", // Mighty Gorgon
		"avwelme0.def", // Earth Elemental
		"avwstone.def", // Magma Elemental
		//
		"avwtrll.def", // Troll
		"avwglmg0.def", // Gold Golem
	},
	// level 6
	{
		"avwcvlr0.def", // Cavalier
		"avwcvlx0.def", // Champion
		"avwunic0.def", // Unicorn
		"avwunix0.def", // War Unicorn
		"avwnaga0.def", // Naga
		"avwnagx0.def", // Naga Queen
		"avwefre0.def", // Efreeti
		"avwefrx0.def", // Efreet Sultan
		"avwbkni0.def", // Black Knight
		"avwbknx0.def", // Dread Knight
		"avwmant0.def", // Manticore
		"avwmanx0.def", // Scorpicore
		"avwcycl0.def", // Cyclops
		"avwcycx0.def", // Cyclops King
		"avwwyvr.def", // Wyvern
		"avwwyvx0.def", // Wyvern Monarch
		"avwpsye.def", // Psychic Elemental
		"avwmagel.def", // Magic Elemental
		//
		"avwglmd0.def", // Diamond Golem
		"avwench.def", // Enchanter
	},
	// level 7
	{
		"avwangl.def", // Angel
		"avwarch.def", // Archangel
		"avwdrag0.def", // Green Dragon
		"avwdrax0.def", // Gold Dragon
		"avwtitn0.def", // Giant
		"avwtitx0.def", // Titan
		"avwdevl0.def", // Devil
		"avwdevx0.def", // Arch Devil
		"avwbone0.def", // Bone Dragon
		"avwbonx0.def", // Ghost Dragon
		"avwrdrg.def", // Red Dragon
		"avwddrx0.def", // Black Dragon
		"avwbhmt0.def", // Behemoth
		"avwbhmx0.def", // Ancient Behemoth
		"avwhydr.def", // Hydra
		"avwhydx0.def", // Chaos Hydra
		"avwfbird.def", // Firebird
		"avwphx.def", // Phoenix
		//
		"avwfdrg.def", // Faerie Dragon
		"avwrust.def", // Rust Dragon
		"avwcdrg.def", // Crystal Dragon
		"avwazure.def", // Azure Dragon
	},
};

const std::map<ETownType, std::vector<std::string> > towns_map =
{
	{
		ETownType::CASTLE,
		{
			"avccast0.def",
			"avccasx0.def",
			"avccasz0.def",
		}
	},
	{
		ETownType::RAMPART,
		{
			"avcramp0.def",
			"avcramx0.def",
			"avcramz0.def",
		}
	},
	{
		ETownType::TOWER,
		{
			"avctowr0.def",
			"avctowx0.def",
			"avctowz0.def",
		}
	},
	{
		ETownType::INFERNO,
		{
			"avcinft0.def",
			"avcinfx0.def",
			"avcinfz0.def",
		}
	},
	{
		ETownType::NECROPOLIS,
		{
			"avcnecr0.def",
			"avcnecx0.def",
			"avcnecz0.def",
		}
	},
	{
		ETownType::DUNGEON,
		{
			"avcdung0.def",
			"avcdunx0.def",
			"avcdunz0.def",
		}
	},
	{
		ETownType::STRONGHOLD,
		{
			"avcstro0.def",
			"avcstrx0.def",
			"avcstrz0.def",
		}
	},
	{
		ETownType::FORTRESS,
		{
			"avcftrt0.def",
			"avcftrx0.def",
			"avcforz0.def",
		}
	},
	{
		ETownType::CONFLUX,
		{
			"avchfor0.def",
			"avchforx.def",
			"avchforz.def",
		}
	},
};

const std::vector<std::string> hero_subtype_appearance_map =
{
	// castle knights
	"ah00_e.def", //   0, orrin
	"ah00_e.def", //   1, valeska
	"ah00_e.def", //   2, edric
	"ah00_e.def", //   3, sylvia
	"ah00_e.def", //   4, lord haart
	"ah00_e.def", //   5, sorsha
	"ah00_e.def", //   6, christian
	"ah00_e.def", //   7, tyris
	// castle clerics
	"ah01_e.def", //   8, rion
	"ah01_e.def", //   9, adela
	"ah01_e.def", //  10, cuthbert
	"ah01_e.def", //  11, adelaide
	"ah01_e.def", //  12, ingham
	"ah01_e.def", //  13, sanya
	"ah01_e.def", //  14, loynis
	"ah01_e.def", //  15, caitlin
	// rampart rangers
	"ah02_e.def", //  16, mephala
	"ah02_e.def", //  17, ufretin
	"ah02_e.def", //  18, jenova
	"ah02_e.def", //  19, ryland
	"ah02_e.def", //  20, thorgrim
	"ah02_e.def", //  21, ivor
	"ah02_e.def", //  22, clancy
	"ah02_e.def", //  23, kyrre
	// rampart druids
	"ah03_e.def", //  24, coronius
	"ah03_e.def", //  25, uland
	"ah03_e.def", //  26, elleshar
	"ah03_e.def", //  27, gem
	"ah03_e.def", //  28, malcom
	"ah03_e.def", //  29, melodia
	"ah03_e.def", //  30, alagar
	"ah03_e.def", //  31, aeris
	// tower alchemists
	"ah04_e.def", //  32, piquedram
	"ah04_e.def", //  33, thane
	"ah04_e.def", //  34, josephine
	"ah04_e.def", //  35, neela
	"ah04_e.def", //  36, torosar
	"ah04_e.def", //  37, fafner
	"ah04_e.def", //  38, rissa
	"ah04_e.def", //  39, iona
	// tower wizards
	"ah05_e.def", //  40, astral
	"ah05_e.def", //  41, halon
	"ah05_e.def", //  42, serena
	"ah05_e.def", //  43, daremyth
	"ah05_e.def", //  44, theodorus
	"ah05_e.def", //  45, solmyr
	"ah05_e.def", //  46, cyra
	"ah05_e.def", //  47, aine
	// inferno demoniacs
	"ah06_e.def", //  48, fiona
	"ah06_e.def", //  49, rashka
	"ah06_e.def", //  50, marius
	"ah06_e.def", //  51, ignatius
	"ah06_e.def", //  52, octavia
	"ah06_e.def", //  53, calh
	"ah06_e.def", //  54, pyre
	"ah06_e.def", //  55, nymus
	// inferno heretics
	"ah07_e.def", //  56, ayden
	"ah07_e.def", //  57, xyron
	"ah07_e.def", //  58, axsis
	"ah07_e.def", //  59, olema
	"ah07_e.def", //  60, calid
	"ah07_e.def", //  61, ash
	"ah07_e.def", //  62, zydar
	"ah07_e.def", //  63, xarfax
	// necropolis deathknights
	"ah08_e.def", //  64, straker
	"ah08_e.def", //  65, vokial
	"ah08_e.def", //  66, moandor
	"ah08_e.def", //  67, charna
	"ah08_e.def", //  68, tamika
	"ah08_e.def", //  69, isra
	"ah08_e.def", //  70, clavius
	"ah08_e.def", //  71, galthran
	// necropolis necromancers
	"ah09_e.def", //  72, septienna
	"ah09_e.def", //  73, aislinn
	"ah09_e.def", //  74, sandro
	"ah09_e.def", //  75, nimbus
	"ah09_e.def", //  76, thant
	"ah09_e.def", //  77, xsi
	"ah09_e.def", //  78, vidomina
	"ah09_e.def", //  79, nagash
	// dungeon overlords
	"ah10_e.def", //  80, lorelei
	"ah10_e.def", //  81, arlach
	"ah10_e.def", //  82, dace
	"ah10_e.def", //  83, ajit
	"ah10_e.def", //  84, damacon
	"ah10_e.def", //  85, gunnar
	"ah10_e.def", //  86, synca
	"ah10_e.def", //  87, shakti
	// dungeon warlocks
	"ah11_e.def", //  88, alamar
	"ah11_e.def", //  89, jaegar
	"ah11_e.def", //  90, malekith
	"ah11_e.def", //  91, jeddite
	"ah11_e.def", //  92, geon
	"ah11_e.def", //  93, deemer
	"ah11_e.def", //  94, sephinroth
	"ah11_e.def", //  95, darkstorn
	// stronghold barbarians
	"ah12_e.def", //  96, yog
	"ah12_e.def", //  97, gurnisson
	"ah12_e.def", //  98, jabarkas
	"ah12_e.def", //  99, shiva
	"ah12_e.def", // 100, gretchin
	"ah12_e.def", // 101, krellion
	"ah12_e.def", // 102, crag hack
	"ah12_e.def", // 103, tyraxor
	// stronghold battlemages
	"ah13_e.def", // 104, gird
	"ah13_e.def", // 105, vey
	"ah13_e.def", // 106, dessa
	"ah13_e.def", // 107, terek
	"ah13_e.def", // 108, zubin
	"ah13_e.def", // 109, gundula
	"ah13_e.def", // 110, oris
	"ah13_e.def", // 111, saurug
	// fortress beastmasters
	"ah14_e.def", // 112, bron
	"ah14_e.def", // 113, drakon
	"ah14_e.def", // 114, wystan
	"ah14_e.def", // 115, tazar
	"ah14_e.def", // 116, alkin
	"ah14_e.def", // 117, korbac
	"ah14_e.def", // 118, gerwulf
	"ah14_e.def", // 119, broghild
	// fortress witches
	"ah15_e.def", // 120, mirlanda
	"ah15_e.def", // 121, rosic
	"ah15_e.def", // 122, voy
	"ah15_e.def", // 123, verdish
	"ah15_e.def", // 124, merist
	"ah15_e.def", // 125, styg
	"ah15_e.def", // 126, andra
	"ah15_e.def", // 127, tiva
	// conflux planeswalkers
	"ah16_e.def", // 128, pasis
	"ah16_e.def", // 129, thunar
	"ah16_e.def", // 130, ignissa
	"ah16_e.def", // 131, lacus
	"ah16_e.def", // 132, monere
	"ah16_e.def", // 133, erdamon
	"ah16_e.def", // 134, fiur
	"ah16_e.def", // 135, kalt
	// conflux elementalists
	"ah17_e.def", // 136, luna
	"ah17_e.def", // 137, brissa
	"ah17_e.def", // 138, ciele
	"ah17_e.def", // 139, labetha
	"ah17_e.def", // 140, inteus
	"ah17_e.def", // 141, aenain
	"ah17_e.def", // 142, gelare
	"ah17_e.def", // 143, grindan
	// special heroes for campaigns
	"ah00_e.def", // 144, sir mullich, knight
	"ah15_e.def", // 145, adrienne, witch
	"ah00_e.def", // 146, catherine, knight
	"ah05_e.def", // 147, dracon, wizard
	"ah02_e.def", // 148, gelu, ranger
	"ah12_e.def", // 149, kilgor, barbarian
	"ah08_e.def", // 150, undead haart, deathknight
	"ah10_e.def", // 151, mutare, overlord
	"ah00_e.def", // 152, roland, knight
	"ah10_e.def", // 153, mutare drake, overlord
	"ah12_e.def", // 154, boragus, barbarian
	"ah06_e.def", // 155, xeron, demoniac
};

const std::vector<std::string> resources_map =
{
	"avtcrys0.def", // crystals
	"avtgems0.def", // gems
	"avtgold0.def", // gold
	"avtmerc0.def", // mercury
	"avtore0.def", // ore
	"avtsulf0.def", // sulfur
	"avtwood0.def", // wood
};

const std::vector<std::pair<std::string, int> > hero_flags_map =
{
	{ "af00.def", 2 },
	{ "af01.def", 2 },
	{ "af02.def", 2 },
	{ "af03.def", 2 },
	{ "af04.def", 2 },
	{ "af05.def", 2 },
	{ "af06.def", 2 },
	{ "af07.def", 2 },
};
