/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "homm3map.h"

#include <ctype.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <tuple>
#include <utility>

#include <QtCore/QMutexLocker>
#include <QtCore/QUrl>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtOpenGL/QOpenGLFramebufferObjectFormat>

#include "vcmi/CCompressedStream.h"
#include "vcmi/CFileInputStream.h"
#include "vcmi/CGTownInstance.h"
#include "vcmi/MapFormatH3M.h"

#include "data_maps.h"
#include "def_file.h"
#include "homm3singleton.h"
#include "random.h"

enum class SpecialTile
{
	none,
	lavatl,
	watrtl,
	clrrvr,
	mudrvr,
	lavrvr,
};

namespace {

const std::map<std::string, std::tuple<SpecialTile, int> > special_tiles_map = {
	{ "lavatl.def", { SpecialTile::lavatl, 9 } },
	{ "watrtl.def", { SpecialTile::watrtl, 84 } },
	{ "clrrvr.def", { SpecialTile::clrrvr, 12 } },
	{ "mudrvr.def", { SpecialTile::mudrvr, 12 } },
	{ "lavrvr.def", { SpecialTile::lavrvr, 9 } },
};

struct MapItem
{
	std::string name;
	int group = 0;
	int special = -1; // player color or ground frame
	size_t total_frames = 1;

	MapItem() = default;

	explicit MapItem(const std::string &l_name)
		: name(l_name)
	{
	}
};

struct MapItemPosition
{
	int placementOrder = 0;
	int x = 0;
	int y = 0;
	bool isHero = false;
	bool isVisitable = false;

	bool operator<(const MapItemPosition &other) const
	{
		return std::tie(other.placementOrder, this->y, this->isHero, this->isVisitable, this->x) < std::tie(this->placementOrder, other.y, other.isHero, other.isVisitable, other.x);
	}
};

int getMapWidth(const std::shared_ptr<CMap> &map)
{
	if (!map)
	{
		return CMapHeader::MAP_SIZE_SMALL;
	}

	return map->width;
}

int getMapHeight(const std::shared_ptr<CMap> &map)
{
	if (!map)
	{
		return CMapHeader::MAP_SIZE_SMALL;
	}

	return map->height;
}

int getMapLevels(const std::shared_ptr<CMap> &map)
{
	if (!map)
	{
		return 1;
	}

	return (map->twoLevel ? 2 : 1);
}

std::tuple<std::string, int, int> getTerrainTile(const std::shared_ptr<CMap> &map, int x, int y, int level)
{
	if ((!map) || (x < 0) || (x >= map->width) || (y < 0) || (y >= map->height) || (level < 0) || (level >= getMapLevels(map)))
	{
		return std::make_tuple("rocktl.def", 0, 0);
	}

	const auto &tile = map->getTile(int3(x, y, level));

	static const std::map<ETerrainType, std::string> terrain_type_to_name_map = {
		{ ETerrainType::DIRT,         "dirttl.def" },
		{ ETerrainType::SAND,         "sandtl.def" },
		{ ETerrainType::GRASS,        "grastl.def" },
		{ ETerrainType::SNOW,         "snowtl.def" },
		{ ETerrainType::SWAMP,        "swmptl.def" },
		{ ETerrainType::ROUGH,        "rougtl.def" },
		{ ETerrainType::SUBTERRANEAN, "subbtl.def" },
		{ ETerrainType::LAVA,         "lavatl.def" },
		{ ETerrainType::WATER,        "watrtl.def" },
		{ ETerrainType::ROCK,         "rocktl.def" },
	};

	auto terrain_type_iter = terrain_type_to_name_map.find(tile.terType);
	if (terrain_type_iter == terrain_type_to_name_map.end())
	{
		return std::make_tuple("rocktl.def", 0, 0);
	}

	return std::make_tuple(terrain_type_iter->second, tile.terView, tile.extTileFlags & 0x03);
}

std::tuple<std::string, int, int> getRiverTile(const std::shared_ptr<CMap> &map, int x, int y, int level)
{
	if ((!map) || (x < 0) || (x >= map->width) || (y < 0) || (y >= map->height) || (level < 0) || (level >= getMapLevels(map)))
	{
		return std::make_tuple(std::string(), 0, 0);
	}

	const auto &tile = map->getTile(int3(x, y, level));

	static const std::map<ERiverType, std::string> river_type_to_name_map = {
		{ ERiverType::CLEAR_RIVER, "clrrvr.def" },
		{ ERiverType::ICY_RIVER,   "icyrvr.def" },
		{ ERiverType::MUDDY_RIVER, "mudrvr.def" },
		{ ERiverType::LAVA_RIVER,  "lavrvr.def" },
	};

	auto river_type_iter = river_type_to_name_map.find(tile.riverType);
	if (river_type_iter == river_type_to_name_map.end())
	{
		return std::make_tuple(std::string(), 0, 0);
	}

	return std::make_tuple(river_type_iter->second, tile.riverDir, (tile.extTileFlags >> 2) & 0x03);
}

std::tuple<std::string, int, int> getRoadTile(const std::shared_ptr<CMap> &map, int x, int y, int level)
{
	if ((!map) || (x < 0) || (x >= map->width) || (y < 0) || (y >= map->height) || (level < 0) || (level >= getMapLevels(map)))
	{
		return std::make_tuple(std::string(), 0, 0);
	}

	const auto &tile = map->getTile(int3(x, y, level));

	static const std::map<ERoadType, std::string> road_type_to_name_map = {
		{ ERoadType::DIRT_ROAD,        "dirtrd.def" },
		{ ERoadType::GRAVEL_ROAD,      "gravrd.def" },
		{ ERoadType::COBBLESTONE_ROAD, "cobbrd.def" },
	};

	auto road_type_iter = road_type_to_name_map.find(tile.roadType);
	if (road_type_iter == road_type_to_name_map.end())
	{
		return std::make_tuple(std::string(), 0, 0);
	}

	return std::make_tuple(road_type_iter->second, tile.roadDir, (tile.extTileFlags >> 4) & 0x03);
}

Def loadDefFile(const std::string &name, int special)
{
	const auto &lod_entries = Homm3MapSingleton::getInstance()->lod_entries;

	auto lod_entries_iter = lod_entries.find(name);
	if (lod_entries_iter == lod_entries.end())
	{
		return Def();
	}

	Def image_def = read_def_file(std::get<0>(lod_entries_iter->second), std::get<1>(lod_entries_iter->second), special);

	static const std::set<DefType> allowed_name_set =
	{
		DefType::spell,
		DefType::sprite,
		DefType::creature,
		DefType::map,
		DefType::map_hero,
		DefType::terrain,
		DefType::cursor,
		DefType::interface,
		DefType::sprite_frame,
		DefType::battle_hero,
	};

	if ((image_def.groups.empty()) || (allowed_name_set.find(image_def.type) == allowed_name_set.end()))
	{
		return Def();
	}

	return image_def;
}

} // unnamed namespace

#define frame_duration 180

bool AnimatedItem::operator<(const AnimatedItem &other) const
{
	return std::tie(this->name, this->group, this->special, this->total_frames, this->is_terrain) < std::tie(other.name, other.group, other.special, other.total_frames, other.is_terrain);
}

Homm3MapLoader::Homm3MapLoader(QObject *parent)
	: QObject(parent)
{
}

void Homm3MapLoader::loadMapData(QString map_name, std::shared_ptr<CMap> map, int level)
{
	std::shared_ptr<MapData> result = std::make_shared<MapData>();

	if (map)
	{
		result->m_map = map;
	}
	else
	{
		try
		{
			QUrl map_url(map_name);
			map_url.setScheme(QLatin1String("file"));

			std::unique_ptr<CInputStream> data_stream(new CCompressedStream(std::unique_ptr<CFileInputStream>(new CFileInputStream(map_url.toLocalFile().toLocal8Bit().data())), true));

			CMapLoaderH3M map_loader(data_stream.get());

			result->m_map = map_loader.loadMap();
		}
		catch (...)
		{
			// ignore
			result->m_map.reset();
		}
	}

	result->m_name = map_name;
	result->m_level = std::min(std::max(level, 0), getMapLevels(result->m_map) - 1);

	// first load all images
	std::map<std::tuple<std::string, int>, Def> defs_map;
	std::map<MapItemPosition, std::vector<MapItem> > map_objects;

	QVector<int> top_edge, right_edge, bottom_edge, left_edge;

	auto load_def_file_func = [&defs_map](const std::string &name, int special) -> Def {
		auto def_iter = defs_map.find(std::make_tuple(name, special));
		if (def_iter == defs_map.end())
		{
			Def def_result = loadDefFile(name, special);

			if (def_result.type != DefType::unknown)
			{
				defs_map[std::make_tuple(name, special)] = def_result;
			}

			return def_result;
		}
		else
		{
			return def_iter->second;
		}
	};

	size_t total_squares = 4 + 2 * getMapWidth(result->m_map) + 2 * getMapHeight(result->m_map);

	// load edges
	{
		Def def_file = load_def_file_func("edg.def", -1);

		if ((def_file.type != DefType::unknown) && (def_file.groups.size() > 0))
		{
			for (int i = 16; i < 36; ++i)
			{
				if (def_file.groups[0].frames.size() > i)
				{
					result->m_texture_atlas.insertItem(TextureItem("edg.def", 0, i, -1), QSize(def_file.fullWidth, def_file.fullHeight));
				}
			}
		}
	}

	if (result->m_map)
	{
		// load terrain, rivers and roads
		for (int tile_y = 0; tile_y < getMapHeight(result->m_map); ++tile_y)
		{
			for (int tile_x = 0; tile_x < getMapWidth(result->m_map); ++tile_x)
			{
				auto tile_info = getTerrainTile(result->m_map, tile_x, tile_y, result->m_level);

				++total_squares;
				Def def_file = load_def_file_func(std::get<0>(tile_info), -1);

				if ((def_file.type != DefType::unknown) && (def_file.groups.size() > 0) && (def_file.groups[0].frames.size() > std::get<1>(tile_info)))
				{
					auto special_tile_iter = special_tiles_map.find(std::get<0>(tile_info));
					if (special_tile_iter == special_tiles_map.end())
					{
						result->m_texture_atlas.insertItem(TextureItem(std::get<0>(tile_info), 0, std::get<1>(tile_info), -1), QSize(def_file.fullWidth, def_file.fullHeight));
					}
					else
					{
						for (int frame = 0; frame < std::get<1>(special_tile_iter->second); ++frame)
						{
							result->m_texture_atlas.insertItem(TextureItem(std::get<0>(tile_info), 0, std::get<1>(tile_info), frame), QSize(def_file.fullWidth, def_file.fullHeight));
						}
					}
				}

				auto river_info = getRiverTile(result->m_map, tile_x, tile_y, result->m_level);
				if (!std::get<0>(river_info).empty())
				{
					++total_squares;
					def_file = load_def_file_func(std::get<0>(river_info), -1);

					if ((def_file.type != DefType::unknown) && (def_file.groups.size() > 0) && (def_file.groups[0].frames.size() > std::get<1>(river_info)))
					{
						auto special_tile_iter = special_tiles_map.find(std::get<0>(river_info));
						if (special_tile_iter == special_tiles_map.end())
						{
							result->m_texture_atlas.insertItem(TextureItem(std::get<0>(river_info), 0, std::get<1>(river_info), -1), QSize(def_file.fullWidth, def_file.fullHeight));
						}
						else
						{
							for (int frame = 0; frame < std::get<1>(special_tile_iter->second); ++frame)
							{
								result->m_texture_atlas.insertItem(TextureItem(std::get<0>(river_info), 0, std::get<1>(river_info), frame), QSize(def_file.fullWidth, def_file.fullHeight));
							}
						}
					}
				}

				auto road_info = getRoadTile(result->m_map, tile_x, tile_y, result->m_level);
				if (!std::get<0>(road_info).empty())
				{
					++total_squares;
					def_file = load_def_file_func(std::get<0>(road_info), -1);

					if ((def_file.type != DefType::unknown) && (def_file.groups.size() > 0) && (def_file.groups[0].frames.size() > std::get<1>(road_info)))
					{
						result->m_texture_atlas.insertItem(TextureItem(std::get<0>(road_info), 0, std::get<1>(road_info), -1), QSize(def_file.fullWidth, def_file.fullHeight));
					}
				}
			}
		}

		for (auto iter = result->m_map->objects.begin(); iter != result->m_map->objects.end(); ++iter)
		{
			if ((((*iter)->ID == Obj::ARTIFACT) && ((*iter)->subID == static_cast<decltype((*iter)->subID)>(ArtifactID::GRAIL))) || ((*iter)->ID == Obj::EVENT))
			{
				// skip grail and events
				continue;
			}

			if ((*iter)->pos.z != result->m_level)
			{
				// skip another map level
				continue;
			}

			MapItemPosition pos;

			pos.x = (*iter)->pos.x;
			pos.y = (*iter)->pos.y;
			pos.placementOrder = (*iter)->appearance.printPriority;

			if (((*iter)->ID == Obj::HERO) || ((*iter)->ID == Obj::RANDOM_HERO) || ((*iter)->ID == Obj::HERO_PLACEHOLDER))
			{
				pos.isHero = true;
			}

			pos.isVisitable = (*iter)->appearance.isVisitable;

			MapItem item { (*iter)->appearance.animationFile };

			// lowercase name
			std::transform(item.name.begin(), item.name.end(), item.name.begin(), [](unsigned char c) { return tolower(c); });

			// towns, dwellings, mines, garrisons, lighthouses, shipyards have owners
			if (((*iter)->ID != Obj::HERO) && ((*iter)->ID != Obj::RANDOM_HERO) && ((*iter)->ID != Obj::HERO_PLACEHOLDER) && (static_cast<int>((*iter)->tempOwner) >= 0) && ((*iter)->tempOwner < PlayerColor::PLAYER_LIMIT_I))
			{
				item.special = static_cast<int>((*iter)->tempOwner);
			}

			++total_squares;
			Def def_file = load_def_file_func(item.name, item.special);

			if ((def_file.type != DefType::unknown) && (def_file.groups.size() > item.group) && (def_file.groups[item.group].frames.size() > 0))
			{
				item.total_frames = def_file.groups[item.group].frames.size();

				for (size_t frame = 0; frame < def_file.groups[item.group].frames.size(); ++frame)
				{
					result->m_texture_atlas.insertItem(TextureItem(item.name, item.group, frame, item.special), QSize(def_file.fullWidth, def_file.fullHeight));
				}
			}

			// heroes have flags, insert flag before hero
			if (((*iter)->ID == Obj::HERO) || ((*iter)->ID == Obj::RANDOM_HERO) || ((*iter)->ID == Obj::HERO_PLACEHOLDER))
			{
				auto index = std::min<int>(std::max<int>(static_cast<int>((*iter)->tempOwner), 0), hero_flags_map.size() - 1);
				MapItem flag_item { hero_flags_map[index].first };
				flag_item.group = hero_flags_map[index].second;

				++total_squares;
				def_file = load_def_file_func(flag_item.name, flag_item.special);

				if ((def_file.type != DefType::unknown) && (def_file.groups.size() > flag_item.group) && (def_file.groups[flag_item.group].frames.size() > 0))
				{
					flag_item.total_frames = def_file.groups[flag_item.group].frames.size();

					for (size_t frame = 0; frame < def_file.groups[flag_item.group].frames.size(); ++frame)
					{
						result->m_texture_atlas.insertItem(TextureItem(flag_item.name, flag_item.group, frame, flag_item.special), QSize(def_file.fullWidth, def_file.fullHeight));
					}
				}

				map_objects[pos].push_back(flag_item);
			}

			map_objects[pos].push_back(item);

			// castles may have heroes
			if (((*iter)->ID == Obj::TOWN) || ((*iter)->ID == Obj::RANDOM_TOWN))
			{
				auto town = dynamic_cast<CGTownInstance*>(*iter);
				if (town != nullptr)
				{
					if (town->hero_type)
					{
						std::stringstream ss;
						ss << "ah" << std::setfill('0') << std::setw(2) << static_cast<int>(*(town->hero_type)) << "_e.def";
						std::string hero_picture = ss.str();

						MapItemPosition hero_pos;

						hero_pos.x = pos.x - 1;
						hero_pos.y = pos.y;
						hero_pos.placementOrder = pos.placementOrder;
						hero_pos.isHero = true;
						hero_pos.isVisitable = false;

						MapItem hero_item { hero_picture };

						def_file = load_def_file_func(hero_item.name, hero_item.special);

						if ((def_file.type != DefType::unknown) && (def_file.groups.size() > hero_item.group) && (def_file.groups[hero_item.group].frames.size() > 0))
						{
							hero_item.total_frames = def_file.groups[hero_item.group].frames.size();

							for (size_t frame = 0; frame < def_file.groups[hero_item.group].frames.size(); ++frame)
							{
								result->m_texture_atlas.insertItem(TextureItem(hero_item.name, hero_item.group, frame, hero_item.special), QSize(def_file.fullWidth, def_file.fullHeight));
							}
						}

						auto index = std::min<int>(std::max<int>(static_cast<int>((*iter)->tempOwner), 0), hero_flags_map.size() - 1);
						MapItem flag_item { hero_flags_map[index].first };
						flag_item.group = hero_flags_map[index].second;

						def_file = load_def_file_func(flag_item.name, flag_item.special);

						if ((def_file.type != DefType::unknown) && (def_file.groups.size() > flag_item.group) && (def_file.groups[flag_item.group].frames.size() > 0))
						{
							flag_item.total_frames = def_file.groups[flag_item.group].frames.size();

							for (size_t frame = 0; frame < def_file.groups[flag_item.group].frames.size(); ++frame)
							{
								result->m_texture_atlas.insertItem(TextureItem(flag_item.name, flag_item.group, frame, flag_item.special), QSize(def_file.fullWidth, def_file.fullHeight));
							}
						}

						// insert flag before hero
						total_squares += 2;
						map_objects[hero_pos].push_back(flag_item);
						map_objects[hero_pos].push_back(hero_item);
					}
				}
			}
		}
	}

	// images loaded, construct texture
	const auto atlas_size = result->m_texture_atlas.getSize();

	{
		result->m_texture_data.resize(atlas_size * atlas_size * 4, 0);

		static const uint8_t transparency_palette[] = { 0x00, 0x40, 0x00, 0x00, 0x80, 0xff, 0x80, 0x40 };

		auto shift_palette_idx_func = [](int base_idx, int current_idx, int total_frames, int current_frame) -> int
		{
			return base_idx + (((total_frames - (current_frame % total_frames)) + (current_idx - base_idx)) % total_frames);
		};

		auto items = result->m_texture_atlas.getAllItems();
		for (auto item = items.first; item != items.second; ++item)
		{
			const std::string &def_name = item->first.name;
			auto group_idx = item->first.group;
			auto frame_idx = item->first.frame;
			auto special_idx = item->first.special;
			auto special_tile_type = SpecialTile::none;
			auto special_frame = -1;

			if (def_name == "invalid")
			{
				continue;
			}

			auto special_tile_iter = special_tiles_map.find(def_name);
			if (special_tile_iter != special_tiles_map.end())
			{
				special_tile_type = std::get<0>(special_tile_iter->second);
				special_frame = special_idx;
				special_idx = -1;
			}

			auto def_iter = defs_map.find(std::tie(def_name, special_idx));
			if ((def_iter != defs_map.end()) && (def_iter->second.groups.size() > group_idx) && (def_iter->second.groups[group_idx].frames.size() > frame_idx))
			{
				const Def &image_def = def_iter->second;
				const DefFrame &frame = image_def.groups[group_idx].frames[frame_idx];

				for (int64_t y = 0; y < frame.height; ++y)
				{
					for (int64_t x = 0; x < frame.width; ++x)
					{
						uint32_t idx = frame.data[y * frame.width + x];

						switch (special_tile_type)
						{
						case SpecialTile::none:
						default:
							break;

						case SpecialTile::lavatl:
							if (idx >= 246 && idx < 246 + 9)
							{
								idx = shift_palette_idx_func(246, idx, 9, special_frame);
							}
							break;

						case SpecialTile::watrtl:
							if (idx >= 229 && idx < 229 + 12)
							{
								idx = shift_palette_idx_func(229, idx, 12, special_frame);
							}
							else if (idx >= 242 && idx < 242 + 14)
							{
								idx = shift_palette_idx_func(242, idx, 14, special_frame);
							}
							break;

						case SpecialTile::clrrvr:
							if (idx >= 183 && idx < 183 + 12)
							{
								idx = shift_palette_idx_func(183, idx, 12, special_frame);
							}
							else if (idx >= 195 && idx < 195 + 6)
							{
								idx = shift_palette_idx_func(195, idx, 6, special_frame);
							}
							break;

						case SpecialTile::mudrvr:
							if (idx >= 228 && idx < 228 + 12)
							{
								idx = shift_palette_idx_func(228, idx, 12, special_frame);
							}
							else if (idx >= 183 && idx < 183 + 6)
							{
								idx = shift_palette_idx_func(183, idx, 6, special_frame);
							}
							else if (idx >= 240 && idx < 240 + 6)
							{
								idx = shift_palette_idx_func(240, idx, 6, special_frame);
							}
							break;

						case SpecialTile::lavrvr:
							if (idx >= 240 && idx < 240 + 9)
							{
								idx = shift_palette_idx_func(240, idx, 9, special_frame);
							}
							break;
						}

						result->m_texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4    ] = image_def.rawPalette[idx * 3];
						result->m_texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 1] = image_def.rawPalette[idx * 3 + 1];
						result->m_texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 2] = image_def.rawPalette[idx * 3 + 2];
						result->m_texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 3] = (idx < sizeof(transparency_palette)) ? transparency_palette[idx] : 0xFF;
					}
				}
			}
		}

		defs_map.clear();
	}

	// now add vertices with texture coordinates
	QRect tex_rect;

	result->m_vertices.reserve(total_squares * 6);
	result->m_texcoords.reserve(total_squares * 6);

	if (result->m_map)
	{
		// draw terrain, rivers
		for (int tile_y = 0; tile_y < getMapHeight(result->m_map); ++tile_y)
		{
			for (int tile_x = 0; tile_x < getMapWidth(result->m_map); ++tile_x)
			{
				auto tile_info = getTerrainTile(result->m_map, tile_x, tile_y, result->m_level);

				int special_terrain_index = -1;

				{
					auto special_terrain_iter = special_tiles_map.find(std::get<0>(tile_info));
					if (special_terrain_iter != special_tiles_map.end())
					{
						special_terrain_index = result->m_current_frames[std::get<1>(special_terrain_iter->second)];

						AnimatedItem item;

						item.name = std::get<0>(tile_info);
						item.group = std::get<1>(tile_info);
						item.total_frames = std::get<1>(special_terrain_iter->second);
						item.is_terrain = true;

						result->m_animated_items[item][std::get<2>(tile_info)].insert(result->m_texcoords.size());
					}
				}

				result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 1) * tile_size, 0));
				result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size, 0));
				result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size, 0));
				result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size, 0));
				result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size, 0));
				result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 2) * tile_size, 0));

				tex_rect = result->m_texture_atlas.findItem(TextureItem(std::get<0>(tile_info), 0, std::get<1>(tile_info), special_terrain_index));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(tile_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(tile_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));

				auto river_info = getRiverTile(result->m_map, tile_x, tile_y, result->m_level);
				if (!std::get<0>(river_info).empty())
				{
					int special_river_index = -1;

					{
						auto special_river_iter = special_tiles_map.find(std::get<0>(river_info));
						if (special_river_iter != special_tiles_map.end())
						{
							special_river_index = result->m_current_frames[std::get<1>(special_river_iter->second)];

							AnimatedItem item;

							item.name = std::get<0>(river_info);
							item.group = std::get<1>(river_info);
							item.total_frames = std::get<1>(special_river_iter->second);
							item.is_terrain = true;

							result->m_animated_items[item][std::get<2>(river_info)].insert(result->m_texcoords.size());
						}
					}

					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 1) * tile_size, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 2) * tile_size, 0));

					tex_rect = result->m_texture_atlas.findItem(TextureItem(std::get<0>(river_info), 0, std::get<1>(river_info), special_river_index));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(river_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(river_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				}
			}
		}

		// draw roads
		for (int tile_y = 0; tile_y < getMapHeight(result->m_map); ++tile_y)
		{
			for (int tile_x = 0; tile_x < getMapWidth(result->m_map); ++tile_x)
			{
				auto road_info = getRoadTile(result->m_map, tile_x, tile_y, result->m_level);
				if (!std::get<0>(road_info).empty())
				{
					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 1) * tile_size + tile_size / 2, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size + tile_size / 2, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size + tile_size / 2, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 1) * tile_size + tile_size / 2, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 1) * tile_size, (tile_y + 2) * tile_size + tile_size / 2, 0));
					result->m_vertices.push_back(QVector3D((tile_x + 2) * tile_size, (tile_y + 2) * tile_size + tile_size / 2, 0));

					tex_rect = result->m_texture_atlas.findItem(TextureItem(std::get<0>(road_info), 0, std::get<1>(road_info), -1));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
					result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + ((std::get<2>(road_info) % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((std::get<2>(road_info) / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size)));
				}
			}
		}

		// draw objects
		for (auto pos_iter = map_objects.begin(); pos_iter != map_objects.end(); ++pos_iter)
		{
			for (auto object_iter = pos_iter->second.begin(); object_iter != pos_iter->second.end(); ++object_iter)
			{
				int frame = 0;

				if (object_iter->total_frames > 1)
				{
					frame = result->m_current_frames[object_iter->total_frames];

					AnimatedItem item;

					item.name = object_iter->name;
					item.group = object_iter->group;
					item.special = object_iter->special;
					item.total_frames = object_iter->total_frames;
					item.is_terrain = false;

					result->m_animated_items[item][0].insert(result->m_texcoords.size());
				}

				tex_rect = result->m_texture_atlas.findItem(TextureItem(object_iter->name, object_iter->group, frame, object_iter->special));

				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size - tex_rect.width(), (pos_iter->first.y + 2) * tile_size - tex_rect.height(), 0));
				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size,                    (pos_iter->first.y + 2) * tile_size - tex_rect.height(), 0));
				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size - tex_rect.width(), (pos_iter->first.y + 2) * tile_size,                     0));
				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size,                    (pos_iter->first.y + 2) * tile_size - tex_rect.height(), 0));
				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size - tex_rect.width(), (pos_iter->first.y + 2) * tile_size,                     0));
				result->m_vertices.push_back(QVector3D((pos_iter->first.x + 2) * tile_size,                    (pos_iter->first.y + 2) * tile_size,                     0));

				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
				result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
			}
		}
	}

	// top left edge
	result->m_vertices.push_back(QVector3D(0, 0, 0));
	result->m_vertices.push_back(QVector3D(tile_size, 0, 0));
	result->m_vertices.push_back(QVector3D(0, tile_size, 0));
	result->m_vertices.push_back(QVector3D(tile_size, 0, 0));
	result->m_vertices.push_back(QVector3D(0, tile_size, 0));
	result->m_vertices.push_back(QVector3D(tile_size, tile_size, 0));

	tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, 16, -1));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));

	// top right edge
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, 0, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, 0, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, 0, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, tile_size, 0));

	tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, 17, -1));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));

	// bottom right edge
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));
	result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));

	tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, 18, -1));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));

	// bottom left edge
	result->m_vertices.push_back(QVector3D(0, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D(tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D(0, (getMapHeight(result->m_map) + 2) * tile_size, 0));
	result->m_vertices.push_back(QVector3D(tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
	result->m_vertices.push_back(QVector3D(0, (getMapHeight(result->m_map) + 2) * tile_size, 0));
	result->m_vertices.push_back(QVector3D(tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));

	tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, 19, -1));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));

	// randomize edges
	top_edge.resize(getMapWidth(result->m_map));
	right_edge.resize(getMapHeight(result->m_map));
	bottom_edge.resize(getMapWidth(result->m_map));
	left_edge.resize(getMapHeight(result->m_map));

	for (auto i = 0; i < getMapWidth(result->m_map); ++i)
	{
		top_edge[i] = CRandomGenerator::instance().nextInt<int>(20, 23);
	}

	for (auto i = 0; i < getMapHeight(result->m_map); ++i)
	{
		right_edge[i] = CRandomGenerator::instance().nextInt<int>(24, 27);
	}

	for (auto i = 0; i < getMapWidth(result->m_map); ++i)
	{
		bottom_edge[i] = CRandomGenerator::instance().nextInt<int>(28, 31);
	}

	for (auto i = 0; i < getMapHeight(result->m_map); ++i)
	{
		left_edge[i] = CRandomGenerator::instance().nextInt<int>(32, 35);
	}

	// top edge
	for (auto i = 0; i < getMapWidth(result->m_map); ++i)
	{
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, 0, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, 0, 0));
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, 0, 0));
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, tile_size, 0));

		tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, top_edge[i], -1));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	}

	// right edge
	for (auto i = 0; i < getMapHeight(result->m_map); ++i)
	{
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (i + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 1) * tile_size, (i + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((getMapWidth(result->m_map) + 2) * tile_size, (i + 2) * tile_size, 0));

		tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, right_edge[i], -1));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	}

	// bottom edge
	for (auto i = 0; i < getMapWidth(result->m_map); ++i)
	{
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, (getMapHeight(result->m_map) + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 1) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D((i + 2) * tile_size, (getMapHeight(result->m_map) + 2) * tile_size, 0));

		tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, bottom_edge[i], -1));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	}

	// left edge
	for (auto i = 0; i < getMapHeight(result->m_map); ++i)
	{
		result->m_vertices.push_back(QVector3D(0, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D(tile_size, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D(0, (i + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D(tile_size, (i + 1) * tile_size, 0));
		result->m_vertices.push_back(QVector3D(0, (i + 2) * tile_size, 0));
		result->m_vertices.push_back(QVector3D(tile_size, (i + 2) * tile_size, 0));

		tex_rect = result->m_texture_atlas.findItem(TextureItem("edg.def", 0, left_edge[i], -1));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
		result->m_texcoords.push_back(QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size)));
	}

	Q_EMIT mapLoaded(result);
}

Homm3MapRenderer::Homm3MapRenderer()
	: QQuickFramebufferObject::Renderer()
	, m_texture_id(0)
	, m_need_update_animation(false)
	, m_need_update_map(false)
{
	QObject::connect(&m_frame_timer, &QTimer::timeout, this, &Homm3MapRenderer::updateFrames, Qt::QueuedConnection);

	initialize();

	m_frame_timer.start(std::chrono::milliseconds(frame_duration));
}

Homm3MapRenderer::~Homm3MapRenderer()
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
	}
}

QOpenGLFramebufferObject* Homm3MapRenderer::createFramebufferObject(const QSize &size)
{
	QOpenGLFramebufferObjectFormat format;
	format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	return new QOpenGLFramebufferObject(size, format);
}

void Homm3MapRenderer::initialize()
{
	initializeOpenGLFunctions();

	const char *vertex_source =
		"attribute highp vec4 vertex;\n"
		"attribute mediump vec2 tex_coord;\n"
		"uniform mediump mat4 matrix;\n"
		"varying mediump vec2 tex_output;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = matrix * vertex;\n"
		"	tex_output = tex_coord;\n"
		"}\n";

	const char *fragment_source =
		"varying mediump vec2 tex_output;\n"
		"uniform sampler2D texture_item;"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_FragColor = texture2D(texture_item, tex_output);\n"
		"}\n";

	m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
	m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
	m_program.link();

	m_vertexAttr = m_program.attributeLocation("vertex");
	m_textureAttr = m_program.attributeLocation("tex_coord");
	m_matrixUniform = m_program.uniformLocation("matrix");
	m_shaderTexture = m_program.uniformLocation("texture_item");

	glUniform1i(m_shaderTexture, 0);

	glGenTextures(1, &m_texture_id);
}

void Homm3MapRenderer::render()
{
	prepareRenderData();

	if (m_need_update_animation)
	{
		m_need_update_animation = false;

		updateAnimatedItems();
	}

	glDepthMask(true);
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 orthoview;
	orthoview.ortho(0, (getMapWidth(m_map) + 2) * tile_size, 0, (getMapHeight(m_map) + 2) * tile_size, -1, 1);

	m_program.bind();
	m_program.setUniformValue(m_matrixUniform, orthoview);

	m_program.enableAttributeArray(m_vertexAttr);
	m_program.enableAttributeArray(m_textureAttr);

	m_program.setAttributeArray(m_vertexAttr, m_vertices.data());
	m_program.setAttributeArray(m_textureAttr, m_texcoords.data());
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

	glDisable(GL_BLEND);
	m_program.disableAttributeArray(m_vertexAttr);
	m_program.disableAttributeArray(m_textureAttr);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_program.release();

	glEnable(GL_DEPTH_TEST);

	update();
}

void Homm3MapRenderer::prepareRenderData()
{
	if (m_need_update_map && !m_texture_data.empty())
	{
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture_atlas.getSize(), m_texture_atlas.getSize(), 0,  GL_RGBA, GL_UNSIGNED_BYTE, m_texture_data.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFinish();
		glBindTexture(GL_TEXTURE_2D, 0);

		m_texture_data.clear();
	}

	m_need_update_map = false;
}

void Homm3MapRenderer::updateFrames()
{
	for (auto iter = m_current_frames.begin(); iter != m_current_frames.end(); ++iter)
	{
		iter->second = (iter->second + 1) % iter->first;
	}

	m_need_update_animation = true;
}

void Homm3MapRenderer::synchronize(QQuickFramebufferObject *item)
{
	auto map_item = static_cast<Homm3Map*>(item);

	m_need_update_map = true;

	QMutexLocker guard(&(map_item->m_data_mutex));

	m_map = map_item->m_map;

	m_vertices = std::move(map_item->m_vertices);
	m_texcoords = std::move(map_item->m_texcoords);

	m_texture_atlas = std::move(map_item->m_texture_atlas);

	m_current_frames = std::move(map_item->m_current_frames);

	m_animated_items = std::move(map_item->m_animated_items);

	m_texture_data = std::move(map_item->m_texture_data);

	map_item->m_vertices.clear();
	map_item->m_texcoords.clear();
	map_item->m_texture_atlas.clear();
	map_item->m_current_frames.clear();
	map_item->m_animated_items.clear();
	map_item->m_texture_data.clear();
}

void Homm3MapRenderer::updateAnimatedItems()
{
	const auto atlas_size = m_texture_atlas.getSize();

	for (auto iter = m_animated_items.begin(); iter != m_animated_items.end(); ++iter)
	{
		if (iter->first.is_terrain)
		{
			auto tex_rect = m_texture_atlas.findItem(TextureItem(iter->first.name, 0, iter->first.group, m_current_frames[iter->first.total_frames]));

			for (auto state_iter = iter->second.begin(); state_iter != iter->second.end(); ++state_iter)
			{
				for (auto coord_iter = state_iter->second.begin(); coord_iter != state_iter->second.end(); ++coord_iter)
				{
					m_texcoords[(*coord_iter)    ] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
					m_texcoords[(*coord_iter) + 1] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
					m_texcoords[(*coord_iter) + 2] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
					m_texcoords[(*coord_iter) + 3] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 0) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
					m_texcoords[(*coord_iter) + 4] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 0) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
					m_texcoords[(*coord_iter) + 5] = QVector2D(static_cast<float>(tex_rect.x() + ((state_iter->first % 2 == 1) ? 0 : tex_rect.width())) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + ((state_iter->first / 2 == 1) ? 0 : tex_rect.height())) / static_cast<float>(atlas_size));
				}
			}
		}
		else
		{
			auto tex_rect = m_texture_atlas.findItem(TextureItem(iter->first.name, iter->first.group, m_current_frames[iter->first.total_frames], iter->first.special));

			for (auto coord_iter = iter->second[0].begin(); coord_iter != iter->second[0].end(); ++coord_iter)
			{
				m_texcoords[(*coord_iter)    ] = QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
				m_texcoords[(*coord_iter) + 1] = QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
				m_texcoords[(*coord_iter) + 2] = QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
				m_texcoords[(*coord_iter) + 3] = QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
				m_texcoords[(*coord_iter) + 4] = QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
				m_texcoords[(*coord_iter) + 5] = QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			}
		}
	}
}

Homm3Map::Homm3Map(QQuickItem *parent)
	: QQuickFramebufferObject(parent)
	, m_scale(1.0)
	, m_map_level(0)
{
	Homm3MapLoader *map_loader = new Homm3MapLoader;
	map_loader->moveToThread(&m_worker_thread);

	QObject::connect(&m_worker_thread, &QThread::finished, map_loader, &QObject::deleteLater);
	QObject::connect(this, &Homm3Map::startLoadingMap, map_loader, &Homm3MapLoader::loadMapData, Qt::QueuedConnection);
	QObject::connect(map_loader, &Homm3MapLoader::mapLoaded, this, &Homm3Map::mapLoaded, Qt::QueuedConnection);

	m_worker_thread.start();
}

Homm3Map::~Homm3Map()
{
	m_worker_thread.quit();
	m_worker_thread.wait();
}

QQuickFramebufferObject::Renderer* Homm3Map::createRenderer() const
{
	return new Homm3MapRenderer();
}

void Homm3Map::loadMap(const QString &filename, int level)
{
	Q_EMIT startLoadingMap(filename, std::shared_ptr<CMap>(), level);
}

void Homm3Map::toggleLevel()
{
	QMutexLocker guard(&m_data_mutex);

	if ((!m_map) || (!m_map->twoLevel))
	{
		return;
	}

	Q_EMIT startLoadingMap(m_current_map, m_map, 1 - m_map_level);
}

void Homm3Map::setDataArchives(const QStringList &files)
{
	Homm3MapSingleton::getInstance()->setDataArchives(files);
}

bool Homm3Map::isMapLoaded() const
{
	QMutexLocker guard(&m_data_mutex);

	return static_cast<bool>(m_map);
}

QString Homm3Map::currentMapName() const
{
	QMutexLocker guard(&m_data_mutex);

	return m_current_map;
}

double Homm3Map::scale() const
{
	return m_scale;
}

void Homm3Map::setScale(double value)
{
	// if m_scale == value, return
	if (std::nextafter(m_scale, std::numeric_limits<double>::lowest()) <= value
		&& std::nextafter(m_scale, std::numeric_limits<double>::max()) >= value)
	{
		return;
	}

	QMutexLocker guard(&m_data_mutex);

	m_scale = value;

	Q_EMIT scaleUpdated(m_scale);

	if (m_map)
	{
		setWidth((getMapWidth(m_map) + 2) * tile_size * m_scale);
		setHeight((getMapHeight(m_map) + 2) * tile_size * m_scale);
	}
}

void Homm3Map::mapLoaded(std::shared_ptr<MapData> data)
{
	QString map_name;
	int map_level = 0;

	{
		QMutexLocker guard(&m_data_mutex);

		if ((!data) || data->m_vertices.empty() || data->m_texcoords.empty() || data->m_texture_data.empty())
		{
			return;
		}

		m_map = std::move(data->m_map);
		m_current_map = std::move(data->m_name);
		m_map_level = std::move(data->m_level);

		m_vertices = std::move(data->m_vertices);
		m_texcoords = std::move(data->m_texcoords);

		m_texture_atlas = std::move(data->m_texture_atlas);

		m_current_frames = std::move(data->m_current_frames);

		m_animated_items = std::move(data->m_animated_items);

		m_texture_data = std::move(data->m_texture_data);

		setWidth((getMapWidth(m_map) + 2) * tile_size * m_scale);
		setHeight((getMapHeight(m_map) + 2) * tile_size * m_scale);

		map_name = m_current_map;
		map_level = m_map_level;
	}

	update();

	Q_EMIT loadingFinished(map_name, map_level);
}
