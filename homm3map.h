/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <memory>
#include <set>
#include <tuple>

#include <QtCore/QAtomicInteger>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtQuick/QQuickFramebufferObject>

#include "vcmi/CMap.h"

#include "map_object.h"
#include "texture_atlas.h"

class Homm3MapRenderer;

class Homm3Map: public QQuickFramebufferObject
{
	Q_OBJECT

public:
	explicit Homm3Map(QQuickItem *parent = nullptr);

	virtual QQuickFramebufferObject::Renderer* createRenderer() const override;

	Q_INVOKABLE void loadMap();

	bool hasMap() const;

	Q_INVOKABLE int getMapWidth() const;
	Q_INVOKABLE int getMapHeight() const;
	Q_INVOKABLE int getMapLevels() const;

	std::tuple<std::string, int, int> getTerrainTile(int x, int y, int level) const;
	std::tuple<std::string, int, int> getRiverTile(int x, int y, int level) const;
	std::tuple<std::string, int, int> getRoadTile(int x, int y, int level) const;

	void updateWidth();
	void updateHeight();

private:
	std::shared_ptr<CMap> m_map;

	mutable Homm3MapRenderer *m_renderer;
};

class Homm3MapRenderer: public QObject, public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	explicit Homm3MapRenderer(const Homm3Map *item);
	~Homm3MapRenderer();

	virtual QOpenGLFramebufferObject* createFramebufferObject(const QSize &size) override;

	void initialize();

	virtual void render() override;

	void mapUpdated();
	void prepareRenderData();

	static Def loadDefFile(const std::string &name, int special);

private Q_SLOT:
	void updateFrames();

private:
	const Homm3Map *m_item;

	QOpenGLShaderProgram m_program;
	int m_vertexAttr = 0;
	int m_textureAttr = 0;
	int m_matrixUniform = 0;
	int m_shaderTexture = 0;
	GLuint m_texture_id = 0;

	QAtomicInteger<int> m_need_update_map;
	QVector<QVector3D> m_vertices;
	QVector<QVector2D> m_texcoords;

	QVector<int> m_top_edge, m_right_edge, m_bottom_edge, m_left_edge;

	QSize m_image_size;
	TextureAtlas m_texture_atlas;

	std::map<size_t, size_t> m_current_frames;

	struct AnimatedItem
	{
		std::string name;
		int group = 0;
		int special = -1;
		size_t total_frames = 1;
		bool is_terrain = false;

		bool operator<(const AnimatedItem &other) const;
	};

	QTimer m_frame_timer;
	QAtomicInteger<int> m_need_update_animation;
	std::map<AnimatedItem, std::map<int, std::set<size_t> > > m_animated_items;

	void updateAnimatedItems();
};
