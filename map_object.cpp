/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "map_object.h"

#include "vcmi/CCompressedStream.h"
#include "vcmi/CFileInputStream.h"
#include "vcmi/MapFormatH3M.h"

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
