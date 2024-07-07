/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <memory>
#include <mutex>

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "vcmi/CMap.h"

#include "globals.h"

class Homm3MapSingleton
{
public:
	static std::shared_ptr<Homm3MapSingleton> getInstance();

	std::map<std::string, std::tuple<std::string, LodEntry> > lod_entries;

	void setDataArchives(const QStringList &files);

private:
	Homm3MapSingleton() = default;

	Homm3MapSingleton(const Homm3MapSingleton &other) = delete;
	Homm3MapSingleton& operator=(const Homm3MapSingleton &other) = delete;

	static std::shared_ptr<Homm3MapSingleton> s_instance;
	static std::mutex s_instance_mutex;
};
