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

#include "globals.h"

#include "vcmi/CBinaryReader.h"

std::vector<LodEntry> read_lod_archive_header(CBinaryReader &reader);
