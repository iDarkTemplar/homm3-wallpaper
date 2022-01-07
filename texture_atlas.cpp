/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "texture_atlas.h"

#include <tuple>

TextureItem::TextureItem(const std::string &l_name, int l_group, int l_frame, int l_special)
	: name(l_name)
	, group(l_group)
	, frame(l_frame)
	, special(l_special)
{
}

bool TextureItem::operator<(const TextureItem &other) const
{
	return std::tie(this->name, this->group, this->frame, this->special) < std::tie(other.name, other.group, other.frame, other.special);
}

TextureAtlas::TextureAtlas()
	: m_size(1024)
	, m_current_height(0)
{
	clear();
}

void TextureAtlas::insertItem(const TextureItem &item, const QSize &size)
{
	// first ensure that it's not allocated yet
	if (itemIsPresent(item))
	{
		return;
	}

	for (;;)
	{
		// check to find if there's line of correct height with enough free space is present
		for (auto lines_iter = m_current_lines.begin(); lines_iter != m_current_lines.end(); ++lines_iter)
		{
			if ((lines_iter->line_height == size.height()) && (m_size - lines_iter->current_width >= size.width()))
			{
				m_texture_items[item] = QRect(QPoint(lines_iter->current_width, lines_iter->line_y), size);
				lines_iter->current_width += size.width();
				return;
			}
		}

		// no free lines found, allocate one
		if (m_size - m_current_height >= size.height())
		{
			m_texture_items[item] = QRect(QPoint(0, m_current_height), size);

			Line newline;
			newline.line_y = m_current_height;
			newline.line_height = size.height();
			newline.current_width = size.width();
			m_current_lines.push_back(newline);

			m_current_height += size.height();
			return;
		}

		// no free height either, increase size and try searching again
		m_size *= 2;
	}
}

bool TextureAtlas::itemIsPresent(const TextureItem &item) const
{
	return (m_texture_items.find(item) != m_texture_items.end());
}

QRect TextureAtlas::findItem(const TextureItem &item) const
{
	auto iter = m_texture_items.find(item);
	if (iter == m_texture_items.end())
	{
		iter = m_texture_items.find(TextureItem("invalid"));
		if (iter == m_texture_items.end())
		{
			return QRect();
		}
	}

	return iter->second;
}

size_t TextureAtlas::getSize() const
{
	return m_size;
}

std::pair<std::map<TextureItem, QRect>::const_iterator, std::map<TextureItem, QRect>::const_iterator> TextureAtlas::getAllItems() const
{
	return std::make_pair(m_texture_items.begin(), m_texture_items.end());
}

void TextureAtlas::clear()
{
	m_size = 1024;
	m_current_height = 0;
	m_current_lines.clear();
	m_texture_items.clear();

	insertItem(TextureItem("invalid"), QSize(tile_size, tile_size));
}
