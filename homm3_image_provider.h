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
	Homm3ImageProvider();

	virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};
