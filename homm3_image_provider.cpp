/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "homm3_image_provider.h"

#include <set>

#include "def_file.h"
#include "homm3singleton.h"
#include "random.h"

namespace {

const size_t edge_used_tiles = 16;
const size_t edge_used_tile_in_row = 4;

} // unnamed namespace

Homm3ImageProvider::Homm3ImageProvider()
	: QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

QImage Homm3ImageProvider::requestImage(const QString &id, QSize *size, const QSize &/*requestedSize*/)
{
	if (id != QStringLiteral("edg.def"))
	{
		if (size != nullptr)
		{
			*size = QSize();
		}

		return QImage();
	}

	const auto &lod_entries = Homm3MapSingleton::getInstance()->lod_entries;

	auto lod_entries_iter = lod_entries.find(id.toLocal8Bit().data());
	if (lod_entries_iter == lod_entries.end())
	{
		if (size != nullptr)
		{
			*size = QSize();
		}

		return QImage();
	}

	Def image_def = read_def_file(std::get<0>(lod_entries_iter->second), std::get<1>(lod_entries_iter->second), -1);

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
		if (size != nullptr)
		{
			*size = QSize();
		}

		return QImage();
	}

	QImage result(image_def.fullWidth * edge_used_tile_in_row, image_def.fullHeight * edge_used_tile_in_row, QImage::Format_ARGB32);

	result.fill(Qt::transparent);

	static const uint8_t transparency_palette[] = { 0x00, 0x40, 0x00, 0x00, 0x80, 0xff, 0x80, 0x40 };

	// randomize tiles location
	std::vector<size_t> remaining_tiles = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	for (size_t index = 0; index < edge_used_tiles; ++index)
	{
		size_t tile_idx = 0;

		if (remaining_tiles.size() > 1)
		{
			tile_idx = CRandomGenerator::instance().nextInt<size_t>(0, remaining_tiles.size() - 1);
		}

		size_t frame_index = remaining_tiles[tile_idx];
		remaining_tiles.erase(remaining_tiles.begin() + tile_idx);

		const DefFrame &frame = image_def.groups[0].frames[frame_index];

		for (int64_t y = 0; y < frame.height; ++y)
		{
			for (int64_t x = 0; x < (int64_t) frame.width; ++x)
			{
				uint8_t idx = frame.data[y * frame.width + x];

				result.setPixelColor(
					image_def.fullWidth * (index % edge_used_tile_in_row) + frame.x + x,
					image_def.fullHeight * (index / edge_used_tile_in_row) + frame.y + y,
					QColor(
						image_def.rawPalette[idx * 3],
						image_def.rawPalette[idx * 3 + 1],
						image_def.rawPalette[idx * 3 + 2],
						(idx < sizeof(transparency_palette)) ? transparency_palette[idx] : 0xFF));
			}
		}
	}

	// return results
	if (size != nullptr)
	{
		*size = QSize(image_def.fullWidth * edge_used_tile_in_row, image_def.fullHeight * edge_used_tile_in_row);
	}

	return result;
}
