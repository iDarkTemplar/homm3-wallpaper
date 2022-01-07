/*
 * CFileInputStream.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <stdint.h>

#include <filesystem>
#include <fstream>

#include "CInputStream.h"

/**
 * A class which provides method definitions for reading a file from the filesystem.
 */
class CFileInputStream: public CInputStream
{
public:
	/**
	 * C-tor. Opens the specified file.
	 *
	 * @param file Path to the file.
	 * @param start - offset from file start where real data starts (e.g file on archive)
	 * @param size - size of real data in file (e.g file on archive) or 0 to use whole file
	 *
	 * @throws std::runtime_error if file wasn't found
	 */
	explicit CFileInputStream(const std::filesystem::path &file, int64_t start = 0, int64_t size = 0);

	/**
	 * Reads n bytes from the stream into the data buffer.
	 *
	 * @param data A pointer to the destination data array.
	 * @param size The number of bytes to read.
	 * @return the number of bytes read actually.
	 */
	int64_t read(uint8_t *data, int64_t size) override;

	/**
	 * Seeks the internal read pointer to the specified position.
	 *
	 * @param position The read position from the beginning.
	 * @return the position actually moved to, -1 on error.
	 */
	int64_t seek(int64_t position) override;

	/**
	 * Gets the current read position in the stream.
	 *
	 * @return the read position. -1 on failure or if the read pointer isn't in the valid range.
	 */
	int64_t tell() override;

	/**
	 * Skips delta numbers of bytes.
	 *
	 * @param delta The count of bytes to skip.
	 * @return the count of bytes skipped actually.
	 */
	int64_t skip(int64_t delta) override;

	/**
	 * Gets the length in bytes of the stream.
	 *
	 * @return the length in bytes of the stream.
	 */
	int64_t getSize() override;

private:
	int64_t dataStart;
	int64_t dataSize;

	/** Native c++ input file stream object. */
	std::ifstream fileStream;
};
