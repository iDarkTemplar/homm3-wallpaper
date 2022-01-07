/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include <stdio.h>
#include <inttypes.h>

#include <filesystem>
#include <stdexcept>

#include "def_file.h"
#include "lod_archive.h"

#include "vcmi/CBinaryReader.h"
#include "vcmi/CFileInputStream.h"

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		printf("Reading archive: %s\n", argv[i]);

		try
		{
			CFileInputStream file_stream(std::filesystem::path(argv[i]));
			CBinaryReader reader(&file_stream);
			std::vector<LodEntry> files = read_lod_archive_header(reader);

			size_t j = 1;
			for (auto iter = files.begin(); iter != files.end(); ++iter, ++j)
			{
				printf("File %zu: %s, size %" PRIu32 ", type 0x%02x\n", j, iter->name.c_str(), iter->full_size, (int) iter->filetype);
			}
		}
		catch (const std::exception &e)
		{
			printf("Caught exception: %s\n", e.what());
		}
	}

	return 0;
}
