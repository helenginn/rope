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

#ifndef __vagabond__Conditions__
#define __vagabond__Conditions__

#include "hnet.h"
#include <map>

template <class Value>
struct Conditions
{
	std::map<void *, Value> _conditions;
	
	Value belief() const
	{
		Value val{};
		hnet::init_unassigned(val);

		for (auto it = _conditions.begin(); it != _conditions.end(); it++)
		{
			val = (Value)(val & it->second);
		}
		
		return val;
	}

	bool has_condition(void *ptr) const
	{
		return _conditions.count(ptr) > 0;
	}

	const Value &condition(void *ptr) const
	{
		return _conditions.at(ptr);
	}

	Value &remove_condition(void *ptr)
	{
		if (has_condition(ptr))
		{
			_conditions.erase(ptr);
		}
	}

	void apply_condition(void *ptr, const Value &value)
	{
		_conditions[ptr] = value;
	}
};

#endif
