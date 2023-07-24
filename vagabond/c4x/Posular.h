// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__Posular__
#define __vagabond__Posular__

#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <vagabond/utils/glm_import.h>


struct Posular
{
	glm::vec3 pos;
	
	Posular()
	{
		pos = {};
	}

	Posular(glm::vec3 a)
	{
		pos = a;
	}
	
	operator glm::vec3() const
	{
		return pos;
	}
	
	std::string str() const
	{
		std::ostringstream ss;
		ss << pos.x << ", " << pos.y << ", " << pos.z;
		return ss.str();
	}

	operator float() const
	{
		return glm::length(pos);
	}

	static size_t comparable_size()
	{
		return 3;
	}

	static Posular unit(const float *result)
	{
		glm::vec3 p;
		p.x = result[0];
		p.y = result[1];
		p.z = result[2];

		return Posular(p);
	}

	static void comparable(const Posular &a, float *result)
	{
		result[0] = a.pos.x;
		result[1] = a.pos.y;
		result[2] = a.pos.z;
	}
	
	friend std::ostream &operator<<(std::ostream& stream, const Posular& a)
	{
		stream << "(" << glm::to_string(a.pos) << ")";
		return stream;
	}

	Posular operator-(const Posular &a)
	{
		Posular ret;
		ret.pos = pos - a.pos;
		return ret;
	}

	Posular &operator-=(const Posular &a)
	{
		pos -= a.pos;
		return *this;
	}

	Posular &operator+=(const Posular &a)
	{
		pos += a.pos;
		return *this;
	}

	Posular &operator/=(const float &a)
	{
		pos /= a;
		return *this;
	}

	float operator*=(const float &a)
	{
		pos *= a;
		return *this;
	}

	Posular operator*(const float &a)
	{
		Posular ret;
		ret.pos = pos * a;
		return ret;
	}

	float operator*(const Posular &a)
	{
		return glm::dot(pos, a.pos);
	}
};

inline bool valid(const Posular &a)
{
	return isfinite(a.pos.x) && (a.pos.x == a.pos.x);
}

#endif
