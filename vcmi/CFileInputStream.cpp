/*
 * CFileInputStream.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include "CFileInputStream.h"

CFileInputStream::CFileInputStream(const std::filesystem::path &file, int64_t start, int64_t size)
	: dataStart{start}
	, dataSize{size}
	, fileStream{file.string(), std::ios::in | std::ios::binary}
{
	if (fileStream.fail())
	{
		throw std::runtime_error("File " + file.string() + " isn't available.");
	}

	if (dataSize == 0)
	{
		dataSize = std::filesystem::file_size(file);
	}

	fileStream.seekg(dataStart, std::ios::beg);
}


int64_t CFileInputStream::read(uint8_t *data, int64_t size)
{
	int64_t origin = tell();
	int64_t toRead = std::min(dataSize - origin, size);
	fileStream.read(reinterpret_cast<char *>(data), toRead);
	return fileStream.gcount();
}

int64_t CFileInputStream::seek(int64_t position)
{
	fileStream.seekg(dataStart + std::min(position, dataSize));
	return tell();
}

int64_t CFileInputStream::tell()
{
	return static_cast<int64_t>(fileStream.tellg()) - dataStart;
}

int64_t CFileInputStream::skip(int64_t delta)
{
	int64_t origin = tell();
	// ensure that we're not seeking past the end of real data
	int64_t toSeek = std::min(dataSize - origin, delta);
	fileStream.seekg(toSeek, std::ios::cur);

	return tell() - origin;
}

int64_t CFileInputStream::getSize()
{
	return dataSize;
}
