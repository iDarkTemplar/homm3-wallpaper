/*
 * int3.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <stdint.h>

#include <tuple>

/// Class which consists of three integer values. Represents position on adventure map.
class int3
{
public:
	int32_t x, y, z;

	//c-tor: x, y, z initialized to 0
	int3() : x(0), y(0), z(0) {} // I think that x, y, z should be left uninitialized.
	//c-tor: x, y, z initialized to i
	explicit int3(const int32_t i) : x(i), y(i), z(i) {}
	//c-tor: x, y, z initialized to X, Y, Z
	int3(const int32_t X, const int32_t Y, const int32_t Z) : x(X), y(Y), z(Z) {}
	int3(const int3 &c) : x(c.x), y(c.y), z(c.z) {} // Should be set to default (C++11)?

	int3& operator=(const int3 &c) // Should be set to default (C++11)?
	{
		x = c.x;
		y = c.y;
		z = c.z;

		return *this;
	}

	int3 operator-() const { return int3(-x, -y, -z); }

	int3 operator+(const int3 &i) const { return int3(x + i.x, y + i.y, z + i.z); }
	int3 operator-(const int3 &i) const { return int3(x - i.x, y - i.y, z - i.z); }

	//returns int3 with coordinates increased by given number
	int3 operator+(const int32_t i) const { return int3(x + i, y + i, z + i); }

	//returns int3 with coordinates decreased by given number
	int3 operator-(const int32_t i) const { return int3(x - i, y - i, z - i); }

	//returns int3 with coordinates multiplied by given number
	int3 operator*(const double i) const { return int3((int)(x * i), (int)(y * i), (int)(z * i)); }

	//returns int3 with coordinates divided by given number
	int3 operator/(const double i) const { return int3((int)(x / i), (int)(y / i), (int)(z / i)); }

	//returns int3 with coordinates multiplied by given number
	int3 operator*(const int32_t i) const { return int3(x * i, y * i, z * i); }

	//returns int3 with coordinates divided by given number
	int3 operator/(const int32_t i) const { return int3(x / i, y / i, z / i); }

	int3& operator+=(const int3 &i)
	{
		x += i.x;
		y += i.y;
		z += i.z;

		return *this;
	}

	int3& operator-=(const int3 &i)
	{
		x -= i.x;
		y -= i.y;
		z -= i.z;

		return *this;
	}

	//increases all coordinates by given number
	int3& operator+=(const int32_t i)
	{
		x += i;
		y += i;
		z += i;
		return *this;
	}

	//decreases all coordinates by given number
	int3& operator-=(const int32_t i)
	{
		x -= i;
		y -= i;
		z -= i;

		return *this;
	}

	bool operator==(const int3 &i) const
	{
		return std::tie(x, y, z) == std::tie(i.x, i.y, i.z);
	}

	bool operator!=(const int3 &i) const
	{
		return std::tie(x, y, z) != std::tie(i.x, i.y, i.z);
	}

	bool operator<(const int3 &i) const
	{
		return std::tie(z, y, x) < std::tie(i.z, i.y, i.x);
	}
};
