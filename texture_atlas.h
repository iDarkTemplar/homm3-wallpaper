/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <stddef.h>

#include <list>
#include <map>
#include <string>
#include <utility>

#include <QtCore/QRect>

#define tile_size 32

struct TextureItem
{
	std::string name;
	int group = 0;
	int frame = 0;
	int special = -1; // player color or ground frame

	TextureItem() = default;
	explicit TextureItem(const std::string &l_name, int l_group = 0, int l_frame = 0, int l_special = -1);

	bool operator<(const TextureItem &other) const;
};

class TextureAtlas
{
public:
	TextureAtlas();

	void insertItem(const TextureItem &item, const QSize &size);

	bool itemIsPresent(const TextureItem &item) const;
	QRect findItem(const TextureItem &item) const;

	size_t getSize() const;

	std::pair<std::map<TextureItem, QRect>::const_iterator, std::map<TextureItem, QRect>::const_iterator> getAllItems() const;

	void clear();

private:
	struct Line
	{
		size_t line_height = 0;
		size_t line_y = 0;
		size_t current_width = 0;
	};

	size_t m_size;
	size_t m_current_height;
	std::list<Line> m_current_lines;

	std::map<TextureItem, QRect> m_texture_items;
};
