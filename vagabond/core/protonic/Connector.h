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

#ifndef __vagabond__Connector__
#define __vagabond__Connector__

#include "hnet.h"
#include "Conditions.h"
#include <functional>
#include <list>

namespace hnet
{

template <typename Value>
struct Connector
{
	Connector()
	{
		init_unassigned(_value);
		_default = _value;
	}
	
	~Connector()
	{

	}

	Connector(const Value &init)
	{
		_value = init;
		_default = _value;
	}

	/* add a constraint check function to the list of functions to run on
	 * an update of value */
	void add_constraint_check(const Checker &checker)
	{
		_checks.push_back(checker);
	}
	
	/* add a forget routine called from a constraint */
	void add_forget(const Forget &forget)
	{
		_forgets.push_back(forget);
	}
	
	void pop_last_check(void *ptr)
	{
		forget(ptr);

		_checks.pop_back();
		_forgets.pop_back();
	}
	
	void forget_all(void *blame)
	{
		forget(blame);
		check_all(blame);
	}
	
	void report()
	{
		std::cout << "===============" << std::endl;
		std::cout << "Report for " << this << std::endl;
		_conditions.report_conditions();
	}

	/* returns true if changed */
	bool assign_value_without_checking(const Value &value, void *informant,
	                                   void *blame)
	{
		bool ignore = false;

		if (_conditions.from_informant_and_blame(informant, blame) == value)
		{
			return ignore; // tbd
		}
		
		Value before = _conditions.belief();
		_conditions.apply_condition(informant, blame, value);
		Value after = _conditions.belief();
		/*
		std::cout << this << " " << before << 
		" before applying condition " << value << 
		" resulting in " << after << std::endl;
		*/
		
		return (before != after);
	}
	
	bool forget(void *blame)
	{
		int total = _conditions.remove_condition_with_blame(blame);
		if (total == 0)
		{
			return false;
		}

		/* if we reassign a new value, we recalculate checks */
		for (Forget &forget_condition : _forgets)
		{
			forget_condition(blame);
		}
		
		return true;
	}
		
	bool check_all(void *blame)
	{
		if (is_contradictory(value()))
		{
			return false;
		}
		
		/* if we reassign a new value, we recalculate checks */
		for (Checker &checker : _checks)
		{
			if (!checker(blame))
			{
				return false;
			}
		}
		
		return true;
	}
	
	bool assign_value(const Value &value, void *informant, void *blame)
	{
		if (assign_value_without_checking(value, informant, blame))
		{
			return check_all(blame);
		}

		return true;
	}

	Value value()
	{
		return _conditions.belief();
	}

	/* list of attached constraint-checking functions */
	std::list<Checker> _checks;
	std::list<Forget> _forgets;

	/* working value associated with this connector */
	Value _value = {};
	Value _default = {};
	
	Conditions<Value> _conditions;
	
	bool _user = false;
};

typedef Connector<Atom::Values> AtomConnector;
typedef Connector<Bond::Values> BondConnector;
typedef Connector<Hydrogen::Values> HydrogenConnector;
typedef Connector<Count::Values> CountConnector;

/* union to store created connectors in a list */
struct AnyConnector
{
	enum Type
	{
		Atom, Bond, Hydrogen, Count
	};
	
	AnyConnector(BondConnector *const &connector)
	{
		_type = Bond;
		_ptr = connector;
	}
	
	AnyConnector(CountConnector *const &connector)
	{
		_type = Count;
		_ptr = connector;
	}
	
	AnyConnector(AtomConnector *const &connector)
	{
		_type = Atom;
		_ptr = connector;
	}
	
	AnyConnector(HydrogenConnector *const &connector)
	{
		_type = Hydrogen;
		_ptr = connector;
	}
	
	void destroy()
	{
		switch (_type)
		{
			case Atom:
			delete static_cast<AtomConnector *>(_ptr);
			break;

			case Bond:
			delete static_cast<BondConnector *>(_ptr);
			break;

			case Count:
			delete static_cast<CountConnector *>(_ptr);
			break;

			case Hydrogen:
			delete static_cast<HydrogenConnector *>(_ptr);
			break;
			
			default: break;
		}
	}
	
	Type _type;
	void *_ptr;
};

};

#endif
