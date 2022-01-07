/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <stdint.h>

#include <array>
#include <string>
#include <vector>

enum class DefType: uint32_t
{
	unknown = 0x00,
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

struct DefFrame {
	std::string frameName;
	uint32_t fullWidth = 0;
	uint32_t fullHeight = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	std::vector<uint8_t> data;
};

struct DefGroup {
	std::vector<DefFrame> frames;
};

struct Def {
	DefType type = DefType::unknown;
	uint32_t fullWidth = 0;
	uint32_t fullHeight = 0;
	std::array<uint8_t, 256 * 3> rawPalette;
	std::vector<DefGroup> groups;
};

struct LodEntry
{
	std::string name;
	uint32_t offset = 0;
	uint32_t full_size = 0;
	uint32_t compressed_size = 0;
	DefType filetype = DefType::unknown;
};
