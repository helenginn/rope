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
	typedef std::map<std::pair<void *, GuiltVersion>, Value> ConditionMap;
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

	Value from_informant_and_blame(void *informant, 
	                               const GuiltVersion &gv) const
	{
		auto from_combo = [informant, gv]
		(const typename ConditionMap::const_iterator &cond)
		{
			return (cond->first.first == informant && 
			        cond->first.second == gv);
		};
		
		return belief_when(from_combo);
	}

	const Value &condition(void *informant, const GuiltVersion &gv) const
	{
		return _conditions.at(std::make_pair(informant, gv));
	}

	int remove_conditions_with_blame(const GuiltVersion &last)
	{
		int count = 0;
		std::unique_lock<std::mutex> lk(_m);
		for (auto it = _conditions.begin(); it != _conditions.end();)
		{
			if (it->first.second >= last)
			{
				it = _conditions.erase(it);
				count++;
			}
			else
			{
				it++;
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

	void apply_condition(void *informant, const GuiltVersion &gv, 
	                     const Value &value)
	{
		std::unique_lock<std::mutex> lk(_m);
		auto blame = std::make_pair(informant, gv);
		if (_conditions.count(blame) == 0)
		{
			_conditions[blame] = value;
		}
		else
		{
			auto before = _conditions[blame];
			_conditions[blame] = (Value)(before & value);
		}
	}
	
};

#endif
