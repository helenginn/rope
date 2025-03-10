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

#ifndef __vagabond__OpVec__
#define __vagabond__OpVec__

#include <iostream>
#include <vector>
#include <set>

template <class Type>
class OpVec : public std::vector<Type>
{
public:
	OpVec() : std::vector<Type>()
	{

	}

	OpVec(size_t num, Type value = {}) : std::vector<Type>(num, value)
	{

	}

	OpVec(const std::vector<Type> &other) : std::vector<Type>(other)
	{

	}
	
	OpVec(const std::set<Type> &other)
	{
		for (const Type &v : other)
		{
			this->push_back(v);
		}
	}
	
	Type sum() const
	{
		Type sum = {};
		for (const Type &v : *this)
		{
			sum += v;
		}

		sum /= (float)this->size();
		return sum;
	}
	
	template <typename Func>
	void do_on_each(Func &func)
	{
		for (Type &v : *this)
		{
			func(v);
		}
	}

	OpVec<Type> operator/(const float &other) const
	{
		OpVec<Type> ret;
		for (const Type &v : *this)
		{
			ret.push_back(v / other);
		}

		return ret;
	}

	OpVec<Type> operator*(const float &other) const
	{
		OpVec<Type> ret;
		for (const Type &v : *this)
		{
			ret.push_back(v * other);
		}

		return ret;
	}

	OpVec<Type> operator+(const OpVec<Type> &other) const
	{
		OpVec<Type> ret;
		for (size_t i = 0; i < this->size(); i++)
		{
			ret.push_back(this->at(i) + other.at(i));
		}

		return ret;
	}

	OpVec<Type> operator-(const OpVec<Type> &other) const
	{
		OpVec<Type> ret;
		for (size_t i = 0; i < this->size(); i++)
		{
			ret.push_back(this->at(i) - other.at(i));
		}

		return ret;
	}

	OpVec<Type> &operator*=(const float &other)
	{
		for (size_t i = 0; i < this->size(); i++)
		{
			(*this)[i] *= other;
		}

		return *this;
	}

	OpVec<Type> &operator-=(const OpVec<Type> &other)
	{
		this->resize(other.size());

		for (size_t i = 0; i < this->size(); i++)
		{
			(*this)[i] -= other[i];
		}

		return *this;
	}

	OpVec<Type> &operator+=(const OpVec<Type> &other)
	{
		this->resize(other.size());

		for (size_t i = 0; i < this->size(); i++)
		{
			(*this)[i] += other[i];
		}

		return *this;
	}

	OpVec<Type> &operator/=(const OpVec<Type> &other)
	{
		this->resize(other.size());

		for (size_t i = 0; i < this->size(); i++)
		{
			(*this)[i] /= other[i];
		}

		return *this;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const OpVec<Type> &vs)
	{
		ss << "opvec(" << vs.size() << "): ";

		int i = 0;
		for (const Type &f : vs)
		{
			ss << f << ", ";
			i++;
			if (i == 10)
			{
				ss << " ...";
				break;
			}
		}

		return ss;
	}

private:

};

#endif
