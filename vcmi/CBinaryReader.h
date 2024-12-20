/*
 * CBinaryReader.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <stdint.h>
#include <string>

class CInputStream;

/**
 * Reads primitive binary values from a underlying stream.
 *
 * The integers which are read are supposed to be little-endian values permanently. They will be
 * converted to big-endian values on big-endian machines.
 */
class CBinaryReader
{
public:
	/**
	 * Default c-tor.
	 */
	CBinaryReader() = default;

	/**
	 * C-tor.
	 *
	 * @param stream The base stream object which serves as the reading input.
	 */
	explicit CBinaryReader(CInputStream *stream);

	/**
	 * Disable copying
	 */
	CBinaryReader(const CBinaryReader &other) = delete;
	CBinaryReader& operator=(const CBinaryReader &other) = delete;

	/**
	 * Gets the underlying stream.
	 *
	 * @return the base stream.
	 */
	CInputStream* getStream();

	/**
	 * Sets the underlying stream.
	 *
	 * @param stream The base stream to set
	 */
	void setStream(CInputStream *stream);

	/**
	 * Reads n bytes from the stream into the data buffer.
	 *
	 * @param data A pointer to the destination data array.
	 * @param size The number of bytes to read.
	 * @return the number of bytes read actually.
	 */
	int64_t read(uint8_t *data, int64_t size);

	/**
	 * Reads integer of various size. Advances the read pointer.
	 *
	 * @return a read integer.
	 *
	 * @throws std::runtime_error if the end of the stream was reached unexpectedly
	 */
	uint8_t readUInt8();
	int8_t readInt8();
	uint16_t readUInt16();
	int16_t readInt16();
	uint32_t readUInt32();
	int32_t readInt32();
	uint64_t readUInt64();
	int64_t readInt64();

	std::string readString();
	void skipString();

	template <size_t N>
	std::string readSizedString()
	{
		char data[N + 1];
		data[N] = 0;

		read(reinterpret_cast<uint8_t*>(data), N);

		return std::string(data);
	}

	inline bool readBool()
	{
		return readUInt8() != 0;
	}

	void skip(int count);

private:
	/**
	 * Reads any integer. Advances the read pointer by its size.
	 *
	 * @return read integer.
	 *
	 * @throws std::runtime_error if the end of the stream was reached unexpectedly
	 */
	template <typename CData>
	CData readInteger();

	/**
	 * Gets a end of stream exception message.
	 *
	 * @param bytesToRead The number of bytes which should be read.
	 * @return the exception message text
	 */
	std::string getEndOfStreamExceptionMsg(long bytesToRead) const;

	/** The underlying base stream */
	CInputStream *stream = nullptr;
};
