/*
 * CBinaryReader.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "CBinaryReader.h"

#include <endian.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "CInputStream.h"

#if __BYTE_ORDER == __BIG_ENDIAN
template <typename CData>
CData readLE(CData data)
{
	auto dataPtr = (char*) &data;
	std::reverse(dataPtr, dataPtr + sizeof(data));
	return data;
}
#else
template <typename CData>
CData readLE(CData data)
{
	return data;
}
#endif

CBinaryReader::CBinaryReader(CInputStream *stream)
	: stream(stream)
{
}

CInputStream* CBinaryReader::getStream()
{
	return stream;
}

void CBinaryReader::setStream(CInputStream *stream)
{
	this->stream = stream;
}

int64_t CBinaryReader::read(uint8_t *data, int64_t size)
{
	int64_t bytesRead = stream->read(data, size);
	if (bytesRead != size)
	{
		throw std::runtime_error(getEndOfStreamExceptionMsg((long)size));
	}

	return bytesRead;
}

template <typename CData>
CData CBinaryReader::readInteger()
{
	CData val;
	stream->read(reinterpret_cast<unsigned char*>(&val), sizeof(val));
	return readLE(val);
}

//FIXME: any way to do this without macro?
#define INSTANTIATE(datatype, methodname) \
datatype CBinaryReader::methodname() \
{ return readInteger<datatype>(); }

// While it is certanly possible to leave only template method
// but typing template parameter every time can be annoying
// and templates parameters can't be resolved by return type
INSTANTIATE(uint8_t, readUInt8)
INSTANTIATE(int8_t, readInt8)
INSTANTIATE(uint16_t, readUInt16)
INSTANTIATE(int16_t, readInt16)
INSTANTIATE(uint32_t, readUInt32)
INSTANTIATE(int32_t, readInt32)
INSTANTIATE(uint64_t, readUInt64)
INSTANTIATE(int64_t, readInt64)

#undef INSTANTIATE

std::string CBinaryReader::readString()
{
	uint32_t len = readUInt32();

	if (len > 0)
	{
		std::string ret;
		ret.resize(len);
		read(reinterpret_cast<uint8_t*>(&ret[0]), len);
		return ret;
	}

	return std::string();

}

void CBinaryReader::skip(int count)
{
	stream->skip(count);
}

std::string CBinaryReader::getEndOfStreamExceptionMsg(long bytesToRead) const
{
	std::stringstream ss;
	ss << "The end of the stream was reached unexpectedly. The stream has a length of " << stream->getSize() << " and the current reading position is "
				<< stream->tell() << ". The client wanted to read " << bytesToRead << " bytes.";

	return ss.str();
}
