/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "plugin.h"

#include "homm3_image_provider.h"
#include "homm3map.h"

void HOMM3MapPlugin::registerTypes(const char *uri)
{
	Q_ASSERT(uri == QLatin1String("homm3map"));

	qmlRegisterType<Homm3Map>("homm3map", 1, 0, "Homm3Map");
}

void HOMM3MapPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
	Q_ASSERT(uri == QLatin1String("homm3map"));

	engine->addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider);
}
