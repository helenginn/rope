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

#ifndef __vagabond__Angular__
#define __vagabond__Angular__

#define _USE_MATH_DEFINES
#include <math.h>
#include <ostream>
#include <vagabond/utils/degrad.h>


struct Angular
{
	float angle;
	bool hyper = false;
	
	Angular()
	{
		angle = 0;
	}

	Angular(float a)
	{
		angle = a;
	}

	std::string str() const
	{
		return std::to_string(angle);
	}
	
	operator float() const
	{
		return angle;
	}

	static size_t comparable_size()
	{
		return 2;
	}

	static Angular unit(const float *result)
	{
		float c = result[0];
		float s = result[1] / c;

		float angle = rad2deg(atan2(c, s));
		return Angular(angle);
	}

	static void comparable(const Angular &a, float *result)
	{
		float s = sin(deg2rad(a - 90.f));
		float c = cos(deg2rad(a - 90.f));

		result[0] = c;
		result[1] = s * c;
	}
	
	friend std::ostream &operator<<(std::ostream& stream, const Angular& a)
	{
		stream << a.str();
		return stream;
		float s = sin(deg2rad(a - 90.f));
		float c = cos(deg2rad(a - 90.f));
		stream << "(" << s << ", " << s*c << ")";
		return stream;
	}

	Angular &operator-=(Angular &a)
	{
		angle -= a.angle;
		return *this;
	}

	Angular &operator-=(float a)
	{
		angle -= a;
		return *this;
	}

	Angular &operator+=(float a)
	{
		angle += a;
		return *this;
	}

	Angular &operator/=(float a)
	{
		angle /= a;
		return *this;
	}

	Angular &operator*=(float a)
	{
		angle *= a;
		return *this;
	}
};

inline bool valid(const Angular &a)
{
	return isfinite(a.angle) && a.angle == a.angle;
}

#endif
