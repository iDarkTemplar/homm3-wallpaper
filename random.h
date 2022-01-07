/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <random>

class CRandomGenerator
{
public:
	CRandomGenerator();

	template <typename T>
	T nextInt(T min, T max)
	{
		return std::uniform_int_distribution<T>(min, max)(m_generator);
	}

	static CRandomGenerator& instance();

private:
	std::mt19937 m_generator;
};
