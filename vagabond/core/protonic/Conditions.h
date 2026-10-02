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
#include <atomic>

/* counts every real change to any connector's belief (a condition that
 * narrows it, or forgotten conditions widening it back) - lets something
 * that only needs to react to connector changes (e.g. Coordinated's
 * realign(), run by every energy term of every scored configuration) tell
 * "nothing at all has changed since I last looked" with one comparison. May
 * over-count, never under-count. */
struct ConditionEpoch
{
	static unsigned long now()
	{
		return _epoch.load(std::memory_order_relaxed);
	}

	static void bump()
	{
		_epoch.fetch_add(1, std::memory_order_relaxed);
	}

	inline static std::atomic<unsigned long> _epoch{0};
};

template <class Value>
struct Conditions
{
	typedef std::map<GuiltVersion, Value> ConditionMap;
	ConditionMap _conditions;
	std::atomic<Value> _cache{};
	bool _written{false};
	
	size_t size() const
	{
		return _conditions.size();
	}
	
	Value calculated_belief() const
	{
		Value val{};
		hnet::init_unassigned(val);

		for (typename ConditionMap::const_iterator it = _conditions.begin(); 
		     it != _conditions.end(); it++)
		{
			val = (Value)(val & it->second);
		}
		
		return val;
	}
	
	// the belief is the AND of every condition, and conditions are only
	// ever narrowed by apply_condition(), so _cache is kept up to date there
	// (and rebuilt when conditions are forgotten) rather than re-ANDing the
	// whole map on every read - value() is read many times per constraint
	// check. cache_only is kept for callers but no longer changes anything:
	// the cache is always current once written.
	Value belief(bool cache_only = false)
	{
		if (!_written)
		{
			_cache = calculated_belief();
			_written = true;
		}

		return _cache;
	}

	int remove_conditions_with_blame(const GuiltVersion &last)
	{
		int count = 0;
		for (auto it = _conditions.begin(); it != _conditions.end();)
		{
			if (it->first >= last)
			{
				it = _conditions.erase(it);
				count++;
			}
			else
			{
				it++;
			}
		}

		if (count > 0)
		{
			_cache = calculated_belief();
			_written = true;
			ConditionEpoch::bump();
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
			std::cout << "Guilt version ";
			std::cout << it->first << " says: " << it->second << std::endl;
		}
		std::cout << "Belief: " << belief() << std::endl;
		std::cout << "===============" << std::endl;
		std::cout << std::endl;

	}

	void apply_condition(void *informant, const GuiltVersion &gv, 
	                     const Value &value)
	{
		if (_conditions.count(gv) == 0)
		{
			_conditions[gv] = value;
		}
		else
		{
			auto before = _conditions[gv];
			_conditions[gv] = (Value)(before & value);
		}

		if (_written)
		{
			Value before = _cache;
			_cache = (Value)(before & value);
			if (_cache.load() != before)
			{
				ConditionEpoch::bump();
			}
		}
		else
		{
			ConditionEpoch::bump();
		}
	}
	
};

#endif
