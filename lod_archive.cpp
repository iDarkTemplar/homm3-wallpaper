/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "lod_archive.h"

#include <string.h>

#include <stdexcept>

std::vector<LodEntry> read_lod_archive_header(CBinaryReader &reader)
{
	std::vector<LodEntry> result;

	const char lod_header[] = "LOD";

	uint8_t file_header[sizeof(lod_header)];

	reader.read(file_header, sizeof(file_header));

	if (memcmp(lod_header, file_header, sizeof(lod_header)) != 0)
	{
		throw std::runtime_error("Invalid LOD header");
	}

	reader.skip(4);

	uint64_t total_files = reader.readUInt32();

	result.reserve(total_files);

	reader.skip(80);

	for (uint64_t i = 0; i < total_files; ++i)
	{
		LodEntry entry;

		entry.name            = reader.readSizedString<16>();
		entry.offset          = reader.readUInt32();
		entry.full_size       = reader.readUInt32();
		entry.filetype        = static_cast<DefType>(reader.readUInt32());
		entry.compressed_size = reader.readUInt32();

		result.push_back(entry);
	}

	return result;
}

