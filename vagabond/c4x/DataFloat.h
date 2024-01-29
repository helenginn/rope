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

#ifndef __vagabond__DataFloat__
#define __vagabond__DataFloat__

#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>

struct DataFloat
{
	DataFloat()
	{
		b = 0;
	}

	DataFloat(const float &f)
	{
		b = f;
	}
	
	operator float() const
	{
		return b;
	}

	std::string str() const
	{
		return std::to_string(b);
	}

	static size_t comparable_size()
	{
		return 1;
	}

	static DataFloat unit(const float *result)
	{
		return DataFloat(result[0]);
	}

	static void comparable(const DataFloat &a, float *result)
	{
		result[0] = a.b;
	}
	
	friend std::ostream &operator<<(std::ostream& stream, const DataFloat& a)
	{
		stream << a.b;
		return stream;
	}

	DataFloat operator-(const DataFloat &a)
	{
		DataFloat ret;
		ret.b = b - a.b;
		return ret;
	}

	DataFloat &operator-=(const DataFloat &a)
	{
		b -= a.b;
		return *this;
	}

	DataFloat &operator+=(const DataFloat &a)
	{
		b += a.b;
		return *this;
	}

	DataFloat &operator/=(const float &a)
	{
		b /= a;
		return *this;
	}

	float operator*=(const float &a)
	{
		b *= a;
		return *this;
	}

	DataFloat operator*(const float &a)
	{
		DataFloat ret;
		ret.b = b * a;
		return ret;
	}

	float operator*(const DataFloat &a)
	{
		return b * a.b;
	}

	float b;
};

inline bool valid(const DataFloat &a)
{
	return isfinite(a.b) && (a.b == a.b);
}

#endif
