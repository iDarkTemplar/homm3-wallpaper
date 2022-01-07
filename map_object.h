/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <memory>
#include <mutex>

#include <QtCore/QObject>

#include "vcmi/CMap.h"

#include "globals.h"

class Homm3MapSingleton
{
public:
	static std::shared_ptr<Homm3MapSingleton> getInstance();

	std::shared_ptr<CMap> map;
	std::shared_ptr<std::map<std::string, std::tuple<std::string, LodEntry> > > lod_entries;

private:
	Homm3MapSingleton();

	Homm3MapSingleton(const Homm3MapSingleton &other) = delete;
	Homm3MapSingleton& operator=(const Homm3MapSingleton &other) = delete;

	static std::shared_ptr<Homm3MapSingleton> s_instance;
	static std::mutex s_instance_mutex;
};

class Homm3MapObject: public QObject
{
	Q_OBJECT

public:
	explicit Homm3MapObject(QObject *parent = nullptr);

	Q_INVOKABLE bool loadMap(const QString &filename);

Q_SIGNALS:
	void mapLoadComplete();

private:
	std::shared_ptr<Homm3MapSingleton> m_singleton;
};
