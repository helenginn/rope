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
#include "Guilt.h"
#include <map>
#include <mutex>

template <class Value>
struct Conditions
{
	typedef std::map<std::pair<void *, void *>, Value> ConditionMap;
	ConditionMap _conditions;
	std::mutex _m;
	
	size_t size() const
	{
		return _conditions.size();
	}
	
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
	
	Value belief(bool *acquired = nullptr)
	{
		if (!acquired)
		{
			std::unique_lock<std::mutex> lk(_m);
			return belief_when([](const typename ConditionMap::const_iterator &) 
			                   { return true; });
		}
		else
		{
			std::unique_lock<std::mutex> lk(_m, std::defer_lock);
			if (lk.try_lock())
			{
				*acquired = true;
				return belief_when([](const typename 
				                      ConditionMap::const_iterator &) 
				{ return true; });
			}
			else
			{
				*acquired = false;
				return (Value)(~Value{});
			}
		}
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

	int remove_conditions_with_blame(const OpSet<void *> &guilts)
	{
		int count = 0;
		std::unique_lock<std::mutex> lk(_m);
		ConditionMap tmp;
		for (auto it = _conditions.begin(); it != _conditions.end(); it++)
		{
			if (guilts.count(it->first.second))
			{
				count++;
			}
			else
			{
				tmp[it->first] = it->second;
			}
		}
		
		_conditions = tmp;
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
		Guilt::guilt().addGuilt(blame);

		std::unique_lock<std::mutex> lk(_m);
		_conditions[std::make_pair(informant, blame)] = value;
	}
	
};

#endif
