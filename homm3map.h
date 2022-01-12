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

#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include <QtCore/QMetaType>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtQuick/QQuickFramebufferObject>

#include "vcmi/CMap.h"

#include "globals.h"
#include "texture_atlas.h"

class Homm3MapRenderer;

struct AnimatedItem
{
	std::string name;
	int group = 0;
	int special = -1;
	size_t total_frames = 1;
	bool is_terrain = false;

	bool operator<(const AnimatedItem &other) const;
};

struct MapData
{
	std::shared_ptr<CMap> m_map;
	int m_level = 0;

	std::vector<QVector3D> m_vertices;
	std::vector<QVector2D> m_texcoords;

	TextureAtlas m_texture_atlas;

	std::map<size_t, size_t> m_current_frames;

	std::map<AnimatedItem, std::map<int, std::set<size_t> > > m_animated_items;

	std::vector<uint8_t> m_texture_data;
};

class Homm3MapLoader: public QObject
{
	Q_OBJECT

public:
	explicit Homm3MapLoader(QObject *parent = nullptr);

Q_SIGNALS:
	void mapLoaded(std::shared_ptr<MapData> data);

public Q_SLOTS:
	void loadMapData(QString map_name, std::shared_ptr<CMap> map, int level);
};

class Homm3Map: public QQuickFramebufferObject
{
	Q_OBJECT

	Q_PROPERTY(double scale READ scale WRITE setScale NOTIFY scaleUpdated);

public:
	explicit Homm3Map(QQuickItem *parent = nullptr);
	~Homm3Map();

	virtual QQuickFramebufferObject::Renderer* createRenderer() const override;

	Q_INVOKABLE void loadMap(const QString &filename, int level);
	Q_INVOKABLE void toggleLevel();
	Q_INVOKABLE void setDataArchives(const QStringList &files);
	Q_INVOKABLE bool isMapLoaded() const;

	double scale() const;
	void setScale(double value);

Q_SIGNALS:
	void loadingFinished();
	void scaleUpdated(double);
	void startLoadingMap(QString map_name, std::shared_ptr<CMap> map, int level);

private Q_SLOTS:
	void mapLoaded(std::shared_ptr<MapData> data);

private:
	QThread m_worker_thread;

	double m_scale;

	mutable QMutex m_data_mutex;

	std::shared_ptr<CMap> m_map;
	int m_map_level;

	std::vector<QVector3D> m_vertices;
	std::vector<QVector2D> m_texcoords;

	TextureAtlas m_texture_atlas;

	std::map<size_t, size_t> m_current_frames;

	std::map<AnimatedItem, std::map<int, std::set<size_t> > > m_animated_items;

	std::vector<uint8_t> m_texture_data;

	friend class Homm3MapRenderer;
};

class Homm3MapRenderer: public QObject, public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	explicit Homm3MapRenderer();
	~Homm3MapRenderer();

	virtual QOpenGLFramebufferObject* createFramebufferObject(const QSize &size) override;

	void initialize();

	virtual void render() override;

	void prepareRenderData();

private Q_SLOTS:
	void updateFrames();

protected:
	virtual void synchronize(QQuickFramebufferObject *item) override;

private:
	QOpenGLShaderProgram m_program;
	int m_vertexAttr = 0;
	int m_textureAttr = 0;
	int m_matrixUniform = 0;
	int m_shaderTexture = 0;
	GLuint m_texture_id = 0;

	std::shared_ptr<CMap> m_map;

	std::vector<QVector3D> m_vertices;
	std::vector<QVector2D> m_texcoords;

	TextureAtlas m_texture_atlas;

	std::map<size_t, size_t> m_current_frames;

	std::map<AnimatedItem, std::map<int, std::set<size_t> > > m_animated_items;

	std::vector<uint8_t> m_texture_data;

	QTimer m_frame_timer;
	bool m_need_update_animation;
	bool m_need_update_map;

	void updateAnimatedItems();
};

Q_DECLARE_METATYPE(std::shared_ptr<MapData>);
Q_DECLARE_METATYPE(std::shared_ptr<CMap>);
