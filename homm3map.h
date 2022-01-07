/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <memory>

#include <QtCore/QObject>
#include <QtQuick/QQuickFramebufferObject>

#include "vcmi/CMap.h"

#include "map_object.h"

class Homm3MapRenderer;

class Homm3Map: public QQuickFramebufferObject
{
	Q_OBJECT

public:
	explicit Homm3Map(QQuickItem *parent = nullptr);

	virtual QQuickFramebufferObject::Renderer* createRenderer() const override;

	Q_INVOKABLE void loadMap();

	Q_INVOKABLE int getMapWidth() const;
	Q_INVOKABLE int getMapHeight() const;
	Q_INVOKABLE int getMapLevels() const;

	void updateWidth();
	void updateHeight();

private:
	std::shared_ptr<CMap> m_map;

	mutable Homm3MapRenderer *m_renderer;
};
