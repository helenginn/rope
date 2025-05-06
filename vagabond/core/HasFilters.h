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

#ifndef __vagabond__HasFilters__
#define __vagabond__HasFilters__

#include <vector>
#include <functional>

template <class Type>
class HasFilters
{
	typedef std::function<bool(const Type &type)> Filter;

protected:
	const Type &filtered(const std::vector<Type> &types, int idx) const
	{
		if (filters.size() == 0)
		{
			return types[idx];
		}

		size_t count = 0;
		for (const Type &type : types)
		{
			if (isFilteredIn(type))
			{
				if (count == idx)
				{
					return type;
				}

				count++;
			}
		}
		
		return types[0];
	}

	size_t filteredCount(const std::vector<Type> &types) const
	{
		if (filters.size() == 0)
		{
			return types.size();
		}

		size_t count = 0;
		for (const Type &type : types)
		{
			if (isFilteredIn(type))
			{
				count++;
			}
		}
		
		return count;
	}

public:
	bool hasFilters() const
	{
		return filters.size() > 0;
	}

	void clearFilters()
	{
		filters.clear();
	}
	
	void addFilter(const Filter &filter)
	{
		filters.push_back(filter);
	}

private:
	// bool true = filtered in, false = filtered out
	std::vector<Filter> filters;
	
	bool isFilteredIn(const Type &type) const
	{
		for (const Filter &filter : filters)
		{
			if (!filter(type))
			{
				return false;
			}
		}

		return true;
	}

};

#endif
