/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "def_file.h"

#include "vcmi/CBinaryReader.h"
#include "vcmi/CCompressedStream.h"
#include "vcmi/CFileInputStream.h"

#include <stdexcept>

namespace {

struct DefGroupHelperData
{
	uint32_t framesCount = 0;
	std::vector<std::string> filenames;
	std::vector<uint32_t> frameOffsets;
	bool is_legacy = false;
};

} // unnamed namespace

Def read_def_file(const std::string &lod_filename, const LodEntry &lod_entry)
{
	Def result;

	std::unique_ptr<CInputStream> data_stream;

	if (lod_entry.compressed_size != 0)
	{
		std::unique_ptr<CFileInputStream> compressed_file_stream(new CFileInputStream(std::filesystem::path(lod_filename), lod_entry.offset, lod_entry.compressed_size));

		data_stream.reset(new CCompressedStream(std::move(compressed_file_stream), false));
	}
	else
	{
		data_stream.reset(new CFileInputStream(std::filesystem::path(lod_filename), lod_entry.offset, lod_entry.full_size));
	}

	CBinaryReader reader(data_stream.get());

	result.type          = static_cast<DefType>(reader.readUInt32());
	result.fullWidth     = reader.readUInt32();
	result.fullHeight    = reader.readUInt32();
	uint32_t groupsCount = reader.readUInt32();

	reader.read(result.rawPalette.data(), result.rawPalette.size());

	result.groups.resize(groupsCount);
	std::vector<DefGroupHelperData> groups_helper_data(groupsCount);

	for (uint64_t group_index = 0; group_index < (uint64_t) groupsCount; ++group_index)
	{
		DefGroupHelperData &group_helper = groups_helper_data[group_index];

		reader.skip(4); /* group type */
		group_helper.framesCount = reader.readUInt32();
		reader.skip(8); /* unknown */

		group_helper.filenames.reserve(group_helper.framesCount);
		group_helper.frameOffsets.reserve(group_helper.framesCount);

		for (uint64_t frame_index = 0; frame_index < (uint64_t) group_helper.framesCount; ++frame_index)
		{
			group_helper.filenames.push_back(reader.readSizedString<13>());
		}

		for (uint64_t frame_index = 0; frame_index < (uint64_t) group_helper.framesCount; ++frame_index)
		{
			group_helper.frameOffsets.push_back(reader.readUInt32());
		}

		auto current_position = data_stream->tell();

		auto frame_offsets_iter = group_helper.frameOffsets.begin();

		for ( ; frame_offsets_iter != group_helper.frameOffsets.end(); ++frame_offsets_iter)
		{
			data_stream->seek(*frame_offsets_iter);
			uint64_t size = reader.readUInt32() + 32;
			uint64_t frameEnd = size + *frame_offsets_iter;

			if (frameEnd > (uint64_t) lod_entry.full_size)
			{
				break;
			}
		}

		group_helper.is_legacy = (frame_offsets_iter != group_helper.frameOffsets.end());
		data_stream->seek(current_position);
	}

	for (uint64_t group_index = 0; group_index < (uint64_t) groupsCount; ++group_index)
	{
		DefGroupHelperData &group_helper = groups_helper_data[group_index];

		result.groups[group_index].frames.resize(group_helper.framesCount);

		for (uint64_t frame_index = 0; frame_index < (uint64_t) group_helper.framesCount; ++frame_index)
		{
			DefFrame &frame = result.groups[group_index].frames[frame_index];

			data_stream->seek(group_helper.frameOffsets[frame_index]);

			frame.frameName = group_helper.filenames[frame_index];

			uint32_t frame_size  = reader.readUInt32();
			uint32_t compression = reader.readUInt32();
			frame.fullWidth      = reader.readUInt32();
			frame.fullHeight     = reader.readUInt32();

			if (group_helper.is_legacy)
			{
				frame.width  = frame.fullWidth;
				frame.height = frame.fullHeight;
				frame.x      = 0;
				frame.y      = 0;
			}
			else
			{
				frame.width  = reader.readUInt32();
				frame.height = reader.readUInt32();
				frame.x      = reader.readUInt32();
				frame.y      = reader.readUInt32();
			}

			auto dataOffset = data_stream->tell();

			switch (compression)
			{
			case 0:
				frame.data.resize(frame_size);
				reader.read(frame.data.data(), frame.data.size());
				break;

			case 1:
				{
					std::vector<uint32_t> offsets(frame.height);

					for (uint64_t offset_index = 0; offset_index < (uint64_t) frame.height; ++offset_index)
					{
						offsets[offset_index] = reader.readUInt32();
					}

					frame.data.reserve(frame_size);

					for (auto offset_iter = offsets.begin(); offset_iter != offsets.end(); ++offset_iter)
					{
						data_stream->seek(dataOffset + *offset_iter);
						uint32_t left = frame.width;

						do
						{
							uint8_t index = reader.readUInt8();
							uint16_t length = static_cast<uint16_t>(reader.readUInt8()) + 1;

							if (index == 0xFF)
							{
								frame.data.resize(frame.data.size() + length);
								reader.read(frame.data.data() + (frame.data.size() - length), length);
							}
							else
							{
								frame.data.insert(frame.data.end(), length, index);
							}

							left -= length;
						} while (left != 0);
					}
				}
				break;

			case 2:
				{
					std::vector<uint16_t> offsets(frame.height);

					for (uint64_t offset_index = 0; offset_index < (uint64_t) frame.height; ++offset_index)
					{
						offsets[offset_index] = reader.readUInt16();
					}

					frame.data.reserve(frame_size);

					for (auto offset_iter = offsets.begin(); offset_iter != offsets.end(); ++offset_iter)
					{
						data_stream->seek(dataOffset + *offset_iter);
						uint32_t left = frame.width;

						do
						{
							uint8_t code = reader.readUInt8();
							uint8_t index = (code >> 5);
							uint8_t length = (code & 0x1F) + 1;

							if (index == 0x07)
							{
								frame.data.resize(frame.data.size() + length);
								reader.read(frame.data.data() + (frame.data.size() - length), length);
							}
							else
							{
								frame.data.insert(frame.data.end(), length, index);
							}

							left -= length;
						} while (left != 0);
					}
				}
				break;

			case 3:
				{
					uint64_t offsets_count = static_cast<uint64_t>(frame.height) * static_cast<uint64_t>(frame.width) / 32;

					std::vector<uint16_t> offsets(offsets_count);

					for (auto offset_iter = offsets.begin(); offset_iter != offsets.end(); ++offset_iter)
					{
						*offset_iter = reader.readUInt16();
					}

					frame.data.reserve(frame_size);

					for (auto offset_iter = offsets.begin(); offset_iter != offsets.end(); ++offset_iter)
					{
						data_stream->seek(dataOffset + *offset_iter);
						uint32_t left = 32;

						do
						{
							uint8_t code = reader.readUInt8();
							uint8_t index = (code >> 5);
							uint8_t length = (code & 0x1F) + 1;

							if (index == 0x07)
							{
								frame.data.resize(frame.data.size() + length);
								reader.read(frame.data.data() + (frame.data.size() - length), length);
							}
							else
							{
								frame.data.insert(frame.data.end(), length, index);
							}

							left -= length;
						} while (left != 0);
					}
				}
				break;

			default:
				throw std::runtime_error("Invalid compression type detected");
			}

			if (frame.data.size() != frame.width * frame.height)
			{
				throw std::runtime_error("Invalid frame data size");
			}
		}
	}

	return result;
}