/*
 * ObjectTemplate.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "ObjectTemplate.h"

#include <iterator>

#include "CBinaryReader.h"

void ObjectTemplate::readMap(CBinaryReader &reader)
{
	animationFile = reader.readString();

	uint8_t visitMask[6];

	reader.skip(6); // block mask

	for (auto &byte: visitMask)
	{
		byte = reader.readUInt8();
	}

	for (size_t i = 0; i < 6; ++i) // 6 rows
	{
		for (size_t j = 0; j < 8; ++j) // 8 columns
		{
			if (((visitMask[i] >> j) & 1 ) != 0)
			{
				isVisitable = true;

				// break out of loop
				i = 6;
				j = 8;
			}
		}
	}

	reader.skip(2);
	reader.skip(2); // terrain mask

	id = Obj(reader.readUInt32());
	subid = reader.readUInt32();
	reader.skip(1); // type
	printPriority = reader.readUInt8() * 100; // to have some space in future

	reader.skip(16);
}
