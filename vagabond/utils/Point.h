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

#ifndef __vagabond__Point__
#define __vagabond__Point__

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <iostream>
#include <set>

template <unsigned int D, typename Type>
class Point
{
public:
	Point() {}
	Point(const float *p, Type value = 0)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
		
		_value = value;
	}
	
	bool is_within_hypersphere(const std::vector<float> &centre, const float &rad)
	{
		float sum = 0;
		for (int i = 0; i < centre.size(); i++)
		{
			float add = centre[i] - _p[i];
			sum += add * add;
		}

		return (sum < rad * rad);
	}
	
	float sqlength() const
	{
		float sum = 0;
		for (int i = 0; i < D; i++)
		{
			sum += _p[i] * _p[i];
		}
		return sum;
	}
	
	Point operator*(const Point &other) const
	{
		Point p;
		for (int i = 0; i < D; i++)
		{
			p._p[i] = _p[i] * other._p[i];
		}
		return p;
	}
	
	Point operator-(const Point &other) const
	{
		Point p;
		for (int i = 0; i < D; i++)
		{
			p._p[i] = _p[i] - other._p[i];
		}
		return p;
	}
	
	operator std::vector<float>() const
	{
		std::vector<float> res(D);
		for (int i = 0; i < D; i++)
		{
			res[i] = _p[i];
		}
		return res;
	}
	
	template <typename T>
	void set_vector(const T &p)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
	}
	
	Point(const std::vector<float> &p, Type value = 0)
	{
		for (unsigned int i = 0; i < D; i++)
		{
			_p[i] = p[i];
		}
		
		_value = value;
	}
	
	const float &scalar(int idx) const
	{
		return _p[idx];
	}
	
	const float &operator[](int idx) const
	{
		return _p[idx];
	}
	
	float &operator[](int idx)
	{
		return _p[idx];
	}
	
	float *p()
	{
		return _p;
	}
	
	Type &v_value()
	{
		return _value;
	}
	
	const Type &value() const
	{
		return _value;
	}
	
	void set_value(const Type &val)
	{
		_value = val;
	}

	friend void to_json(json &j, const Point<D, Type> &value);
	friend void from_json(const json &j, Point<D, Type> &value);
private:
	float _p[D]{};
	Type _value{};

	unsigned int _d = D;

	friend std::ostream &operator<<(std::ostream &ss, const Point &p)
	{
		ss << "(";
		for (size_t i = 0; i < p._d; i++)
		{
			ss << p._p[i];
			if (i < p._d - 1)
			{
				ss << ", ";
			}
		}
		ss << ") = " << p._value << " (this=" << &p << ")" << std::endl;
		return ss;
	}
};

inline void to_json(json &j, const Point<2, float> &point)
{
	std::vector<float> coords = point;
	float value = point.value();

	j["coords"] = coords;
	j["val"] = value;
}

inline void from_json(const json &j, Point<2, float> &point)
{
	if (j.count("coords"))
	{
		std::vector<float> coords = j.at("coords");
		point.set_vector(coords);
	}

	if (j.count("val"))
	{
		point.set_value(j.at("val"));
	}
}

#endif
