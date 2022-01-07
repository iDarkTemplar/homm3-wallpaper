/*
 * CCompressedStream.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <vector>

#include "CInputStream.h"

struct z_stream_s;

/// Abstract class that provides buffer for one-directional input streams (e.g. compressed data)
/// Used for zip archives support and in .lod deflate compression
class CBufferedStream: public CInputStream
{
public:
	CBufferedStream() = default;

	/**
	 * Reads n bytes from the stream into the data buffer.
	 *
	 * @param data A pointer to the destination data array.
	 * @param size The number of bytes to read.
	 * @return the number of bytes read actually.
	 *
	 * @throws std::runtime_error if the file decompression was not successful
	 */
	int64_t read(uint8_t *data, int64_t size) override;

	/**
	 * Seeks the internal read pointer to the specified position.
	 * This will cause decompressing data till this position is found
	 *
	 * @param position The read position from the beginning.
	 * @return the position actually moved to, -1 on error.
	 */
	int64_t seek(int64_t position) override;

	/**
	 * Gets the current read position in the stream.
	 *
	 * @return the read position.
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
	 * Causes complete data decompression
	 *
	 * @return the length in bytes of the stream.
	 */
	int64_t getSize() override;

protected:
	/**
	 * @brief virtual method to get more data into the buffer
	 * @return amount of bytes actually read, non-positive values are considered to be end-of-file mark
	 */
	virtual int64_t readMore(uint8_t *data, int64_t size) = 0;

	/// resets all internal state
	void reset();

private:
	/// ensures that buffer contains at lest size of bytes. Calls readMore() to fill remaining part
	void ensureSize(int64_t size);

	/** buffer with already decompressed data */
	std::vector<uint8_t> buffer;

	/** Current read position */
	int64_t position = 0;

	bool endOfFileReached = false;
};

/**
 * A class which provides method definitions for reading a gzip-compressed file
 * This class implements lazy loading - data will be decompressed (and cached) only by request
 */
class CCompressedStream: public CBufferedStream
{
public:
	/**
	 * C-tor.
	 *
	 * @param stream - stream with compresed data
	 * @param gzip - this is gzipp'ed file e.g. campaign or maps, false for files in lod
	 */
	CCompressedStream(std::unique_ptr<CInputStream> stream, bool gzip);

	~CCompressedStream() = default;

	/**
	 * Prepare stream for decompression of next block (e.g. next part of h3c)
	 * Applicable only for streams that contain multiple concatenated compressed data
	 *
	 * @return false if next block was not found, true othervice
	 */
	bool getNextBlock();

private:
	/**
	 * Decompresses data to ensure that buffer has newSize bytes or end of stream was reached
	 */
	int64_t readMore(uint8_t *data, int64_t size) override;

	/** The file stream with compressed data. */
	std::unique_ptr<CInputStream> gzipStream;

	/** buffer with not yet decompressed data*/
	std::vector<uint8_t> compressedBuffer;

	/** struct with current zlib inflate state */
	std::unique_ptr<z_stream_s, void(*)(z_stream_s*)> inflateState;

	enum EState
	{
		ERROR_OCCURED,
		INITIALIZED,
		IN_PROGRESS,
		STREAM_END,
		FINISHED
	};
};
