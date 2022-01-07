/*
 * CInputStream.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CStream.h"

#include <memory>

/**
 * Abstract class which provides method definitions for reading from a stream.
 */
class CInputStream: public virtual CStream
{
public:
	/**
	 * D-tor.
	 */
	virtual ~CInputStream() = default;

	/**
	 * Reads n bytes from the stream into the data buffer.
	 *
	 * @param data A pointer to the destination data array.
	 * @param size The number of bytes to read.
	 * @return the number of bytes read actually.
	 */
	virtual int64_t read(uint8_t *data, int64_t size) = 0;

	/**
	 * @brief for convenience, reads whole stream at once
	 *
	 * @return pair, first = raw data, second = size of data
	 */
	std::pair<std::unique_ptr<uint8_t[]>, int64_t> readAll()
	{
		std::unique_ptr<uint8_t[]> data(new uint8_t[getSize()]);

		seek(0);
		auto readSize = read(data.get(), getSize());

		return std::make_pair(std::move(data), readSize);
	}
};
