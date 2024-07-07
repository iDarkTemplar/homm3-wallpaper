/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "homm3singleton.h"

#include <QtCore/QUrl>

#include "vcmi/CBinaryReader.h"
#include "vcmi/CFileInputStream.h"

#include "lod_archive.h"

std::shared_ptr<Homm3MapSingleton> Homm3MapSingleton::s_instance;
std::mutex Homm3MapSingleton::s_instance_mutex;

std::shared_ptr<Homm3MapSingleton> Homm3MapSingleton::getInstance()
{
	std::lock_guard<std::mutex> instance_lock(s_instance_mutex);

	if (!s_instance)
	{
		s_instance = std::shared_ptr<Homm3MapSingleton>(new Homm3MapSingleton);
	}

	return s_instance;
}

void Homm3MapSingleton::setDataArchives(const QStringList &files)
{
	std::map<std::string, std::tuple<std::string, LodEntry> > new_lod_entries;

	for (const auto &file: files)
	{
		try
		{
			QUrl file_url(file);
			file_url.setScheme(QLatin1String("file"));

			std::string filename = file_url.toLocalFile().toLocal8Bit().data();
			CFileInputStream file_stream{std::filesystem::path(filename)};
			CBinaryReader reader(&file_stream);
			std::vector<LodEntry> parsed_lod_entries = read_lod_archive_header(reader);

			for (auto iter = parsed_lod_entries.begin(); iter != parsed_lod_entries.end(); ++iter)
			{
				new_lod_entries[iter->name] = std::tie(filename, *iter);
			}
		}
		catch (...)
		{
			// ignore
		}
	}

	lod_entries = std::move(new_lod_entries);
}
