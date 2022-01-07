/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <QtCore/QString>
#include <QtQuick/QQuickImageProvider>

#include "globals.h"

class Homm3ImageProvider: public QQuickImageProvider
{
public:
	explicit Homm3ImageProvider(const std::map<std::string, std::tuple<std::string, LodEntry> > &lod_entries);

	virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
	const std::map<std::string, std::tuple<std::string, LodEntry> > &m_lod_entries;
};
