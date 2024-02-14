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

#ifndef __vagabond__Limit__
#define __vagabond__Limit__

#include "hnet.h"
#include <climits>

namespace hnet
{
/* simple constant class to impose a value (AND) on a connector */
template <bool Max>
struct Limit
{
	Limit(CountConnector &informant, CountConnector &affected) :
	_informant(informant), _affected(affected)
	{
		auto self_check = [this](void *prev) { return check(prev); };

		/* add this object's constraint check function to the connector */
		_informant.add_constraint_check(self_check);
		_affected.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };

		_affected.add_forget(forget_me);
		_informant.add_forget(forget_me);

		if (!check(this))
		{
			_affected.pop_last_check(this);
			_informant.pop_last_check(this);

			throw std::runtime_error("New limit immediately "\
			                         "failed validation check");

		}
	}
	
	void forget(void *blame)
	{
		_affected.forget(blame);
		_informant.forget(blame);
	}
	
	void min_max_values(int &min, int &max)
	{
		std::vector<int> values = possible_values(_informant.value());
		min = INT_MAX;
		max = -INT_MAX;

		for (const int &val : values)
		{
			if (val > max) max = val;
			if (val < min) min = val;
		}
	}
	
	bool check(void *previous)
	{
		int min, max;
		min_max_values(min, max);

		if (Max)
		{
			min = -7;
		}
		else 
		{
			max = 7;
		}
		
		std::vector<int> acceptable;
		for (int i = min; i <= max; i++)
		{
			acceptable.push_back(i);
		}

		Count::Values onehot = values_as_count(acceptable);
		
		_affected.assign_value(onehot, this, previous);
		return !is_contradictory(_affected.value());
	}
	
	CountConnector &_informant;
	CountConnector &_affected;
};

typedef Limit<true> MaxLimit;
typedef Limit<false> MinLimit;
};

#endif
