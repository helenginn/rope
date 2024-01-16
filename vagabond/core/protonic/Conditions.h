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
	typedef std::map<std::pair<void *, void *>, Value> ConditionMap;
	ConditionMap _conditions;
	
	template <typename FilterIn>
	Value belief_when(const FilterIn &filtered) const
	{
		Value val{};
		hnet::init_unassigned(val);

		for (typename ConditionMap::const_iterator it = _conditions.begin(); 
		     it != _conditions.end(); it++)
		{
			if (filtered(it))
			{
				val = (Value)(val & it->second);
			}
		}
		
		return val;
	}
	
	Value belief() const
	{
		return belief_when([](const typename ConditionMap::const_iterator &) 
		                   { return true; });
	}

	Value from_informant(void *informant) const
	{
		auto condition_from_informant = 
		[informant](const typename ConditionMap::const_iterator &cond)
		{
			return (cond->first.first == informant);
		};
		
		return belief_when(condition_from_informant);
	}

	Value from_informant_and_blame(void *informant, void *blame) const
	{
		auto from_combo = [informant, blame]
		(const typename ConditionMap::const_iterator &cond)
		{
			return (cond->first.first == informant && 
			        cond->first.second == blame);
		};
		
		return belief_when(from_combo);
	}

	const Value &condition(void *informant, void *blame) const
	{
		return _conditions.at(std::make_pair(informant, blame));
	}

	int remove_condition_with_blame(void *blame)
	{
		bool success = false;
		int count = 0;
		while (!success)
		{
			success = true;
			for (auto it = _conditions.begin(); it != _conditions.end(); it++)
			{
				if (it->first.second == blame)
				{
					_conditions.erase(it->first);
					success = false;
					count++;
					break;
				}
			}
		}
		
		return count;
	}
	
	void report_conditions()
	{
		using namespace hnet;
		std::cout << "===============" << std::endl;
		std::cout << "conditions: " << _conditions.size() << std::endl;
		for (auto it = _conditions.begin(); it != _conditions.end(); it++)
		{
			std::cout << "informant " << it->first.first << " who blames ";
			std::cout << it->first.second << " says: " << it->second << std::endl;
		}
		std::cout << "Belief: " << belief() << std::endl;
		std::cout << "===============" << std::endl;
		std::cout << std::endl;

	}

	void apply_condition(void *informant, void *blame, const Value &value)
	{
		_conditions[std::make_pair(informant, blame)] = value;
	}
};

#endif
