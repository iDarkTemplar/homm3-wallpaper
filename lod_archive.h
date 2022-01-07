/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <stdint.h>

#include <istream>
#include <string>
#include <vector>

#include "vcmi/CBinaryReader.h"

enum class LodFileType
{
	spell = 0x40,
	sprite = 0x41,
	creature = 0x42,
	map = 0x43,
	map_hero = 0x44,
	terrain = 0x45,
	cursor = 0x46,
	interface = 0x47,
	sprite_frame = 0x48,
	battle_hero = 0x49
};

struct LodEntry
{
	std::string name;
	uint32_t offset;
	uint32_t full_size;
	uint32_t compressed_size;
	LodFileType filetype;

	LodEntry();
};

std::vector<LodEntry> read_lod_archive_header(CBinaryReader &reader);
