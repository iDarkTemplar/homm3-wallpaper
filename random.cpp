/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "random.h"

CRandomGenerator& CRandomGenerator::instance()
{
	static CRandomGenerator s_instance;

	return s_instance;
}

CRandomGenerator::CRandomGenerator()
	: m_generator(std::random_device{}())
{
}
