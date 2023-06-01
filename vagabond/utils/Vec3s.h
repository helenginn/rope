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

#ifndef __vagabond__Vec3s__
#define __vagabond__Vec3s__

#include "glm_import.h"

class Vec3s : public std::vector<glm::vec3>
{
public:
	Vec3s operator/(const float &other) const
	{
		Vec3s ret;
		for (const glm::vec3 &v : *this)
		{
			ret.push_back(v / other);
		}

		return ret;
	}

	Vec3s operator*(const float &other) const
	{
		Vec3s ret;
		for (const glm::vec3 &v : *this)
		{
			ret.push_back(v * other);
		}

		return ret;
	}

	Vec3s operator+(const Vec3s &other) const
	{
		Vec3s ret;
		for (size_t i = 0; i < size(); i++)
		{
			ret.push_back(this->at(i) + other.at(i));
		}

		return ret;
	}

	Vec3s operator-(const Vec3s &other) const
	{
		Vec3s ret;
		for (size_t i = 0; i < size(); i++)
		{
			ret.push_back(this->at(i) - other.at(i));
		}

		return ret;
	}

	Vec3s &operator*=(const float &other)
	{
		for (size_t i = 0; i < size(); i++)
		{
			(*this)[i] *= other;
		}

		return *this;
	}

	Vec3s &operator+=(const Vec3s &other)
	{
		resize(other.size());

		for (size_t i = 0; i < size(); i++)
		{
			(*this)[i] += other[i];
		}

		return *this;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const Vec3s &vs)
	{
		std::cout << "vec3s(" << vs.size() << ")" << std::endl;
		return ss;
	}
private:

};

#endif
