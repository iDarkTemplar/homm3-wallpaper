/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "homm3map.h"

#include <map>
#include <tuple>

#include <QtCore/QAtomicInteger>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>

#include "vcmi/MapFormatH3M.h"

#include "def_file.h"
#include "map_object.h"
#include "random.h"
#include "texture_atlas.h"

#define tile_size 32

class Homm3MapRenderer: public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
	explicit Homm3MapRenderer(const Homm3Map *item);
	~Homm3MapRenderer();

	virtual QOpenGLFramebufferObject* createFramebufferObject(const QSize &size) override;

	void initialize();

	virtual void render() override;

	void mapUpdated();
	void prepareRenderData();

	Def loadDefFile(const std::string &name, int special);

private:
	const Homm3Map *m_item;

	QOpenGLShaderProgram m_program;
	int m_vertexAttr = 0;
	int m_textureAttr = 0;
	int m_matrixUniform = 0;
	int m_shaderTexture = 0;
	GLuint m_texture_id = 0;

	QAtomicInteger<int> m_need_update;
	QVector<QVector3D> m_vertices;
	QVector<QVector2D> m_texcoords;

	QVector<int> m_top_edge, m_right_edge, m_bottom_edge, m_left_edge;

	QSize m_image_size;
	TextureAtlas m_texture_atlas;
};

Homm3MapRenderer::Homm3MapRenderer(const Homm3Map *item)
	: QQuickFramebufferObject::Renderer()
	, m_item(item)
	, m_texture_id(0)
	, m_need_update(1)
{
	initialize();
}

Homm3MapRenderer::~Homm3MapRenderer()
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
	}
}

QOpenGLFramebufferObject* Homm3MapRenderer::createFramebufferObject(const QSize &size)
{
	QOpenGLFramebufferObjectFormat format;
	format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	m_image_size = size;

	return new QOpenGLFramebufferObject(size, format);
}

void Homm3MapRenderer::initialize()
{
	initializeOpenGLFunctions();

	const char *vertex_source =
		"attribute highp vec4 vertex;\n"
		"attribute mediump vec2 tex_coord;\n"
		"uniform mediump mat4 matrix;\n"
		"varying mediump vec2 tex_output;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = matrix * vertex;\n"
		"	tex_output = tex_coord;\n"
		"}\n";

	const char *fragment_source =
		"varying mediump vec2 tex_output;\n"
		"uniform sampler2D texture_item;"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_FragColor = texture2D(texture_item, tex_output);\n"
		"}\n";

	m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
	m_program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
	m_program.link();

	m_vertexAttr = m_program.attributeLocation("vertex");
	m_textureAttr = m_program.attributeLocation("tex_coord");
	m_matrixUniform = m_program.uniformLocation("matrix");
	m_shaderTexture = m_program.uniformLocation("texture_item");

	glUniform1i(m_shaderTexture, 0);

	glGenTextures(1, &m_texture_id);

	prepareRenderData();
}

void Homm3MapRenderer::render()
{
	glDepthMask(true);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 orthoview;
	orthoview.ortho(0, (m_item->getMapWidth() + 2) * tile_size, 0, (m_item->getMapHeight() + 2) * tile_size, -1, 1);

	m_program.bind();
	m_program.setUniformValue(m_matrixUniform, orthoview);

	m_program.enableAttributeArray(m_vertexAttr);
	m_program.enableAttributeArray(m_textureAttr);

	m_program.setAttributeArray(m_vertexAttr, m_vertices.constData());
	m_program.setAttributeArray(m_textureAttr, m_texcoords.constData());
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

	m_program.disableAttributeArray(m_vertexAttr);
	m_program.disableAttributeArray(m_textureAttr);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_program.release();
}

void Homm3MapRenderer::mapUpdated()
{
	m_need_update.storeRelease(1);
}

void Homm3MapRenderer::prepareRenderData()
{
	int need_update = m_need_update.fetchAndStoreOrdered(0);
	if (need_update)
	{
		// first load all images
		std::map<std::tuple<std::string, int>, Def> defs_map;

		// load edges
		for (int i = 16; i < 36; ++i)
		{
			Def def_file;

			{
				auto def_iter = defs_map.find(std::make_tuple("edg.def", -1));
				if (def_iter == defs_map.end())
				{
					def_file = loadDefFile("edg.def", -1);

					if (def_file.type != DefType::unknown)
					{
						defs_map[std::make_tuple("edg.def", -1)] = def_file;
					}
				}
				else
				{
					def_file = def_iter->second;
				}
			}

			if ((def_file.type != DefType::unknown) && (def_file.groups.size() > 0) && (def_file.groups[0].frames.size() > i))
			{
				m_texture_atlas.insertItem(TextureItem("edg.def", 0, i, -1), QSize(def_file.fullWidth, def_file.fullHeight));
			}
		}

		// images loaded, construct texture
		const auto atlas_size = m_texture_atlas.getSize();

		{
			QVector<unsigned char> texture_data(atlas_size * atlas_size * 4, 0);

			static const uint8_t transparency_palette[] = { 0x00, 0x40, 0x00, 0x00, 0x80, 0xff, 0x80, 0x40 };

			auto items = m_texture_atlas.getAllItems();
			for (auto item = items.first; item != items.second; ++item)
			{
				const std::string &def_name = item->first.name;
				auto group_idx = item->first.group;
				auto frame_idx = item->first.frame;
				auto special_idx = item->first.special;

				auto def_iter = defs_map.find(std::tie(def_name, special_idx));
				if ((def_iter != defs_map.end()) && (def_iter->second.groups.size() > group_idx) && (def_iter->second.groups[group_idx].frames.size() > frame_idx))
				{
					const Def &image_def = def_iter->second;
					const DefFrame &frame = image_def.groups[group_idx].frames[frame_idx];

					for (int64_t y = 0; y < frame.height; ++y)
					{
						for (int64_t x = 0; x < frame.width; ++x)
						{
							uint8_t idx = frame.data[y * frame.width + x];

							texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4    ] = image_def.rawPalette[idx * 3];
							texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 1] = image_def.rawPalette[idx * 3 + 1];
							texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 2] = image_def.rawPalette[idx * 3 + 2];
							texture_data[((item->second.y() + frame.y + y) * atlas_size + item->second.x() + frame.x + x) * 4 + 3] = (idx < sizeof(transparency_palette)) ? transparency_palette[idx] : 0xFF;
						}
					}
				}
			}

			glBindTexture(GL_TEXTURE_2D, m_texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_size, atlas_size, 0,  GL_RGBA, GL_UNSIGNED_BYTE, texture_data.constData());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFinish();
			glBindTexture(GL_TEXTURE_2D, 0);
			defs_map.clear();
		}

		// now add vertices with texture coordinates
		m_vertices.clear();
		m_texcoords.clear();

		// top left edge
		m_vertices << QVector3D(0, 0, 0);
		m_vertices << QVector3D(tile_size, 0, 0);
		m_vertices << QVector3D(0, tile_size, 0);
		m_vertices << QVector3D(tile_size, 0, 0);
		m_vertices << QVector3D(0, tile_size, 0);
		m_vertices << QVector3D(tile_size, tile_size, 0);

		QRect tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, 16, -1));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));

		// top right edge
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, 0, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, 0, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, 0, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, tile_size, 0);

		tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, 17, -1));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));

		// bottom right edge
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);
		m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);

		tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, 18, -1));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));

		// bottom left edge
		m_vertices << QVector3D(0, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D(tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D(0, (m_item->getMapHeight() + 2) * tile_size, 0);
		m_vertices << QVector3D(tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
		m_vertices << QVector3D(0, (m_item->getMapHeight() + 2) * tile_size, 0);
		m_vertices << QVector3D(tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);

		tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, 19, -1));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));

		// randomize edges
		m_top_edge.resize(m_item->getMapWidth());
		m_right_edge.resize(m_item->getMapHeight());
		m_bottom_edge.resize(m_item->getMapWidth());
		m_left_edge.resize(m_item->getMapHeight());

		for (auto i = 0; i < m_item->getMapWidth(); ++i)
		{
			m_top_edge[i] = CRandomGenerator::instance().nextInt<int>(20, 23);
		}

		for (auto i = 0; i < m_item->getMapHeight(); ++i)
		{
			m_right_edge[i] = CRandomGenerator::instance().nextInt<int>(24, 27);
		}

		for (auto i = 0; i < m_item->getMapWidth(); ++i)
		{
			m_bottom_edge[i] = CRandomGenerator::instance().nextInt<int>(28, 31);
		}

		for (auto i = 0; i < m_item->getMapHeight(); ++i)
		{
			m_left_edge[i] = CRandomGenerator::instance().nextInt<int>(32, 35);
		}

		// top edge
		for (auto i = 0; i < m_item->getMapWidth(); ++i)
		{
			m_vertices << QVector3D((i + 1) * tile_size, 0, 0);
			m_vertices << QVector3D((i + 2) * tile_size, 0, 0);
			m_vertices << QVector3D((i + 1) * tile_size, tile_size, 0);
			m_vertices << QVector3D((i + 2) * tile_size, 0, 0);
			m_vertices << QVector3D((i + 1) * tile_size, tile_size, 0);
			m_vertices << QVector3D((i + 2) * tile_size, tile_size, 0);

			tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, m_top_edge[i], -1));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		}

		// right edge
		for (auto i = 0; i < m_item->getMapHeight(); ++i)
		{
			m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (i + 1) * tile_size, 0);
			m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (i + 1) * tile_size, 0);
			m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (i + 2) * tile_size, 0);
			m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (i + 1) * tile_size, 0);
			m_vertices << QVector3D((m_item->getMapWidth() + 1) * tile_size, (i + 2) * tile_size, 0);
			m_vertices << QVector3D((m_item->getMapWidth() + 2) * tile_size, (i + 2) * tile_size, 0);

			tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, m_right_edge[i], -1));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		}

		// bottom edge
		for (auto i = 0; i < m_item->getMapWidth(); ++i)
		{
			m_vertices << QVector3D((i + 1) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
			m_vertices << QVector3D((i + 2) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
			m_vertices << QVector3D((i + 1) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);
			m_vertices << QVector3D((i + 2) * tile_size, (m_item->getMapHeight() + 1) * tile_size, 0);
			m_vertices << QVector3D((i + 1) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);
			m_vertices << QVector3D((i + 2) * tile_size, (m_item->getMapHeight() + 2) * tile_size, 0);

			tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, m_bottom_edge[i], -1));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		}

		// left edge
		for (auto i = 0; i < m_item->getMapHeight(); ++i)
		{
			m_vertices << QVector3D(0, (i + 1) * tile_size, 0);
			m_vertices << QVector3D(tile_size, (i + 1) * tile_size, 0);
			m_vertices << QVector3D(0, (i + 2) * tile_size, 0);
			m_vertices << QVector3D(tile_size, (i + 1) * tile_size, 0);
			m_vertices << QVector3D(0, (i + 2) * tile_size, 0);
			m_vertices << QVector3D(tile_size, (i + 2) * tile_size, 0);

			tex_rect = m_texture_atlas.findItem(TextureItem("edg.def", 0, m_left_edge[i], -1));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y())                     / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x())                    / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
			m_texcoords << QVector2D(static_cast<float>(tex_rect.x() + tex_rect.width()) / static_cast<float>(atlas_size), static_cast<float>(tex_rect.y() + tex_rect.height()) / static_cast<float>(atlas_size));
		}
	}
}

Def Homm3MapRenderer::loadDefFile(const std::string &name, int special)
{
	const auto &lod_entries = Homm3MapSingleton::getInstance()->lod_entries;

	auto lod_entries_iter = lod_entries->find(name);
	if (lod_entries_iter == lod_entries->end())
	{
		return Def();
	}

	Def image_def = read_def_file(std::get<0>(lod_entries_iter->second), std::get<1>(lod_entries_iter->second), special);

	static const std::set<DefType> allowed_name_set =
	{
		DefType::spell,
		DefType::sprite,
		DefType::creature,
		DefType::map,
		DefType::map_hero,
		DefType::terrain,
		DefType::cursor,
		DefType::interface,
		DefType::sprite_frame,
		DefType::battle_hero,
	};

	if ((image_def.groups.empty()) || (allowed_name_set.find(image_def.type) == allowed_name_set.end()))
	{
		return Def();
	}

	return image_def;
}

Homm3Map::Homm3Map(QQuickItem *parent)
	: QQuickFramebufferObject(parent)
	, m_renderer(nullptr)
{
	loadMap();
}

QQuickFramebufferObject::Renderer* Homm3Map::createRenderer() const
{
	m_renderer = new Homm3MapRenderer(this);

	return m_renderer;
}

void Homm3Map::loadMap()
{
	m_map = Homm3MapSingleton::getInstance()->map;

	updateWidth();
	updateHeight();

	if (m_renderer)
	{
		m_renderer->mapUpdated();
	}
}

int Homm3Map::getMapWidth() const
{
	if (!m_map)
	{
		return CMapHeader::MAP_SIZE_SMALL;
	}

	return m_map->width;
}

int Homm3Map::getMapHeight() const
{
	if (!m_map)
	{
		return CMapHeader::MAP_SIZE_SMALL;
	}

	return m_map->height;
}

int Homm3Map::getMapLevels() const
{
	if (!m_map)
	{
		return 1;
	}

	return (m_map->twoLevel ? 2 : 1);
}

void Homm3Map::updateWidth()
{
	setWidth((getMapWidth() + 2) * tile_size);
}

void Homm3Map::updateHeight()
{
	setHeight((getMapHeight() + 2) * tile_size);
}
