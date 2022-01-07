/*
 * CCompressedStream.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include "CCompressedStream.h"

#include <limits>

#include <zlib.h>

#include <boost/lexical_cast.hpp>

static const int inflateBlockSize = 10000;

int64_t CBufferedStream::read(uint8_t *data, int64_t size)
{
	ensureSize(position + size);

	auto start = buffer.data() + position;
	int64_t toRead = std::min<int64_t>(size, buffer.size() - position);

	std::copy(start, start + toRead, data);
	position += toRead;
	return size;
}

int64_t CBufferedStream::seek(int64_t position)
{
	ensureSize(position);
	this->position = std::min<int64_t>(position, buffer.size());
	return this->position;
}

int64_t CBufferedStream::tell()
{
	return position;
}

int64_t CBufferedStream::skip(int64_t delta)
{
	return seek(position + delta) - delta;
}

int64_t CBufferedStream::getSize()
{
	int64_t prevPos = tell();
	seek(std::numeric_limits<int64_t>::max());
	int64_t size = tell();
	seek(prevPos);
	return size;
}

void CBufferedStream::ensureSize(int64_t size)
{
	while (((int64_t)buffer.size() < size) && (!endOfFileReached))
	{
		int64_t initialSize = buffer.size();
		int64_t currentStep = std::min<int64_t>(size, buffer.size());
		currentStep = std::max<int64_t>(currentStep, 1024); // to avoid large number of calls at start

		buffer.resize(initialSize + currentStep);

		int64_t readSize = readMore(buffer.data() + initialSize, currentStep);
		if (readSize != currentStep)
		{
			endOfFileReached = true;
			buffer.resize(initialSize + readSize);
			buffer.shrink_to_fit();
			return;
		}
	}
}

void CBufferedStream::reset()
{
	buffer.clear();
	position = 0;
	endOfFileReached = false;
}

CCompressedStream::CCompressedStream(std::unique_ptr<CInputStream> stream, bool gzip)
	: CBufferedStream()
	, gzipStream(std::move(stream))
	, compressedBuffer(inflateBlockSize)
	, inflateState(new z_stream, [](z_stream *ptr) { if (ptr != nullptr) { inflateEnd(ptr); delete ptr; } })
{
	// Allocate inflate state
	inflateState->zalloc = Z_NULL;
	inflateState->zfree = Z_NULL;
	inflateState->opaque = Z_NULL;
	inflateState->avail_in = 0;
	inflateState->next_in = Z_NULL;

	int wbits = 15;
	if (gzip)
	{
		wbits += 16;
	}

	int ret = inflateInit2(inflateState.get(), wbits);
	if (ret != Z_OK)
	{
		throw std::runtime_error("Failed to initialize inflate!\n");
	}
}

int64_t CCompressedStream::readMore(uint8_t *data, int64_t size)
{
	if (!inflateState)
	{
		return 0; //file already decompressed
	}

	bool fileEnded = false; //end of file reached
	bool endLoop = false;

	int decompressed = inflateState->total_out;

	inflateState->avail_out = (uInt)size;
	inflateState->next_out = data;

	do
	{
		if (inflateState->avail_in == 0)
		{
			//inflate ran out of available data or was not initialized yet
			// get new input data and update state accordingly
			int64_t availSize = gzipStream->read(compressedBuffer.data(), compressedBuffer.size());
			if (availSize != compressedBuffer.size())
			{
				gzipStream.reset();
			}

			inflateState->avail_in = (uInt)availSize;
			inflateState->next_in  = compressedBuffer.data();
		}

		int ret = inflate(inflateState.get(), Z_NO_FLUSH);

		if ((inflateState->avail_in) == 0 && (gzipStream == nullptr))
		{
			fileEnded = true;
		}

		switch (ret)
		{
		case Z_OK: //input data ended/ output buffer full
			endLoop = false;
			break;
		case Z_STREAM_END: // stream ended. Note that campaign files consist from multiple such streams
			endLoop = true;
			break;
		case Z_BUF_ERROR: // output buffer full. Can be triggered?
			endLoop = true;
			break;
		default:
			if (inflateState->msg == nullptr)
			{
				throw std::runtime_error("Decompression error. Return code was " + boost::lexical_cast<std::string>(ret));
			}
			else
			{
				throw std::runtime_error(std::string("Decompression error: ") + inflateState->msg);
			}
		}
	} while ((!endLoop) && (inflateState->avail_out != 0));

	decompressed = inflateState->total_out - decompressed;

	// Clean up and return
	if (fileEnded)
	{
		inflateState.reset();
	}

	return decompressed;
}

bool CCompressedStream::getNextBlock()
{
	if (!inflateState)
	{
		return false;
	}

	if (inflateReset(inflateState.get()) < 0)
	{
		return false;
	}

	reset();

	return true;
}
