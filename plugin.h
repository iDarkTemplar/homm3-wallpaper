/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <QtQml/QQmlExtensionPlugin>

class HOMM3MapPlugin: public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "homm3map")

public:
	virtual void registerTypes(const char *uri) override;
	virtual void initializeEngine(QQmlEngine *engine, const char *uri) override;
};
