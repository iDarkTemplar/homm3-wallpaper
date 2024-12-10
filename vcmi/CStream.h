/*
 * CStream.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <stdint.h>

class CStream
{
public:
	/**
	 * C-tor.
	 */
	CStream() = default;

	/**
	 * D-tor.
	 */
	virtual ~CStream() = default;

	/**
	 * Disable copying
	 */
	CStream(const CStream &other) = delete;
	CStream& operator=(const CStream &other) = delete;

	/**
	 * Seeks to the specified position.
	 *
	 * @param position The position from the beginning.
	 * @return the position actually moved to, -1 on error.
	 */
	virtual int64_t seek(int64_t position) = 0;

	/**
	 * Gets the current position in the stream.
	 *
	 * @return the position.
	 */
	virtual int64_t tell() = 0;

	/**
	 * Relative seeks to the specified position.
	 *
	 * @param delta The count of bytes to seek from current position.
	 * @return the count of bytes skipped actually.
	 */
	virtual int64_t skip(int64_t delta) = 0;

	/**
	 * Gets the length of the stream.
	 *
	 * @return the length in bytes
	 */
	virtual int64_t getSize() = 0;
};
