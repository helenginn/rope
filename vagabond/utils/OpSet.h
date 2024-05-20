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

#ifndef __vagabond__OpSet__
#define __vagabond__OpSet__

#include <iostream>
#include <vector>
#include <set>

template <class Type>
class OpSet : public std::set<Type>
{
public:
	OpSet() : std::set<Type>()
	{

	}

	OpSet<Type>(const std::set<Type> &other) : std::set<Type>(other)
	{

	}

	OpSet<Type>(const Type &member) : std::set<Type>()
	{
		this->insert(member);
	}

	OpSet<Type>(const std::vector<Type> &other) : std::set<Type>()
	{
		for (const Type &t : other)
		{
			this->insert(t);
		}
	}
	
	/*
	bool operator<(const OpSet<Type> &other) const
	{
		auto it = other.begin();
		for (const Type &ours : *this)
		{
			const Type &theirs = *it;
			if (ours != theirs)
			{
				return ours < theirs;
			}
			
			it++;
			if (it == other.end())
			{
				return false;
			}
		}

		if (it != other.end())
		{
			return true;
		}

		return false;
	}
	*/
	
	template <class Container>
	OpSet<std::pair<Type, Type>> pairs_with(const Container &other)
	{
		OpSet<std::pair<Type, Type>> pairs;

		for (const Type &left : *this)
		{
			for (const Type &right : other)
			{
				pairs.insert({left, right});
			}
		}
		
		return pairs;
	}
	
	std::vector<Type> toVector()
	{
		std::vector<Type> result;

		for (const Type &t : *this)
		{
			result.push_back(t);
		}
		
		return result;
	}

	template <typename Other, class Convert>
	OpSet<Other> convert_to(const Convert &convert)
	{
		OpSet<Other> converted;

		for (Type type : *this)
		{
			Other other = convert(type);
			converted.insert(other);
		}

		return converted;
	}

	template <class Filter>
	void filter(const Filter &filter)
	{
		OpSet<Type> filtered;

		for (Type t : *this)
		{
			if (filter(t))
			{
				filtered.insert(t);
			}
		}

		*this = filtered;
	}


	template <typename Container>
	OpSet<Type> operator+(const Container &other) const
	{
		OpSet<Type> ret(*this);
		for (const Type &t : other)
		{
			ret.insert(t);
		}

		return ret;
	}

	OpSet<Type> &operator+=(const Type &other)
	{
		this->insert(other);
		return *this;
	}

	template <typename Container>
	OpSet<Type> &operator+=(const Container &other)
	{
		for (const Type &t : other)
		{
			this->insert(t);
		}

		return *this;
	}

	template <typename Container>
	OpSet<Type> operator-(const Container &other) const
	{
		OpSet<Type> ret(*this);
		for (const Type &t : other)
		{
			if (ret.count(t))
			{
				ret.erase(t);
			}
		}

		return ret;
	}

	template <typename Container>
	OpSet<Type> &operator-=(const Container &other)
	{
		for (const Type &t : other)
		{
			if (this->count(t))
			{
				this->erase(t);
			}
		}

		return *this;
	}

	OpSet<Type> &operator-=(const Type &member)
	{
		if (this->count(member))
		{
			this->erase(member);
		}

		return *this;
	}
private:

};

#endif
