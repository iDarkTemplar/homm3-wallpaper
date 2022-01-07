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

Homm3MapSingleton::Homm3MapSingleton()
{
}

Homm3MapObject::Homm3MapObject(QObject *parent)
	: QObject(parent)
	, m_singleton(Homm3MapSingleton::getInstance())
{
}

bool Homm3MapObject::loadMap(const QString &filename)
{
	std::unique_ptr<CInputStream> data_stream(new CCompressedStream(std::unique_ptr<CFileInputStream>(new CFileInputStream(filename.toLocal8Bit().data())), true));

	CMapLoaderH3M map_loader(data_stream.get());

	m_singleton->map = map_loader.loadMap();

	Q_EMIT mapLoadComplete();

	return static_cast<bool>(m_singleton->map);
}
