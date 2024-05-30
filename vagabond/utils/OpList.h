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

#ifndef __vagabond__OpList__
#define __vagabond__OpList__

#include <iostream>
#include <list>

template <class Type>
class OpList : public std::list<Type>
{
public:
	OpList() : std::list<Type>()
	{

	}

	OpList(size_t num, Type value = {}) : std::list<Type>(num, value)
	{

	}

	OpList(const std::list<Type> &other) : std::list<Type>(other)
	{

	}
	
	template <typename Func>
	void do_on_each(const Func &func)
	{
		for (Type &v : *this)
		{
			func(v);
		}
	}

	OpList<Type> operator+(const OpList<Type> &other) const
	{
		OpList<Type> ret = *this;
		for (const Type &type : other)
		{
			ret.push_back(type);
		}

		return ret;
	}

	OpList<Type> &operator+=(const OpList<Type> &other)
	{
		for (const Type &type : other)
		{
			this->push_back(type);
		}

		return *this;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const OpList<Type> &vs)
	{
		ss << "oplist(" << vs.size() << "): ";

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
