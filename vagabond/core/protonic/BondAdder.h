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

#ifndef __vagabond__BondAdder__
#define __vagabond__BondAdder__

#include <vector>
#include <sstream>
#include "Constraint.h"

namespace hnet
{
/* addition of all strong bonds from an atom and output of a total sum */
template <Bond::Values Request>
struct BondAdder
{
	void get_certains_maybes(int &total, int &certain, int &maybe)
	{
		total = 0;
		certain = 0;
		maybe = 0;

		for (BondConnector *const &bond : _bonds)
		{
			Bond::Values val = bond->value();
			if (value_is_requested_and_nothing_else(val))
			{
				certain++;
			}
			else if (value_is_not_just_requested(val))
			{
				maybe++;
			}

			total++;
		}
	}
	
	BondAdder(const std::vector<BondConnector *> &bonds, CountConnector &sum)
	: _bonds(bonds), _sum(sum)
	{
		auto self_check = [this](void *prev) { return check(prev); };

		for (BondConnector *const &bc : bonds)
		{
			bc->add_constraint_check(self_check);
		}

		_sum.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };

		for (BondConnector *const &bc : bonds)
		{
			bc->add_forget(forget_me);
		}

		if (!check(this))
		{
			int total = 0; int certain = 0; int maybe = 0;
			get_certains_maybes(total, certain, maybe);

			std::cout << _bonds.size() << " bonds for adder." << std::endl;
			std::cout << total << " bonds of which " << certain << " are "\
			"certainly " << Request << " and " << maybe << " maybe" << std::endl;
			
			for (BondConnector *const &bc : bonds)
			{
				bc->pop_last_check(this);
			}

			_sum.pop_last_check(this);

			std::ostringstream ss;
			ss << Request;

//			std::cout << "Report for bonds of adder " << this << std::endl;
//			for (BondConnector *const &bc : bonds)
			{
//				bc->report();
			}
			throw std::runtime_error("New addition of BondAdder (" + 
			                         ss.str() + ") immediately "\
			                         "failed validation check");
		}
	}
	
	void forget(void *blame)
	{
		_sum.forget(blame);

		for (BondConnector *const &bc : _bonds)
		{
			bc->forget(blame);
		}
	}
	
	bool value_is_not_just_requested(const Bond::Values &val)
	{
		Bond::Values not_request = (Bond::Values)(Bond::Unassigned & ~Request);
		bool request = val & Request;
		bool other = ((val & not_request) != Bond::Contradiction);
		return request && other;
	}
	
	bool value_is_requested_and_nothing_else(const Bond::Values &val)
	{
		Bond::Values not_request = (Bond::Values)(Bond::Unassigned & ~Request);
		return ((val & Request) && (val & not_request) == Bond::Contradiction);
	}
	
	void tell_maybe_bonds(const Bond::Values &tell, void *previous)
	{
		std::vector<BondConnector *> chosen;

		for (BondConnector *const &bond : _bonds)
		{
			Bond::Values val = bond->value();
			/* go on, tell them they're not requested! */
			if (value_is_not_just_requested(val))
			{
				bool changed = bond->assign_value(tell, this, previous);
				if (changed)
				{
					chosen.push_back(bond);
				}
			}
		}
	}
	
	bool check(void *previous)
	{
		int total = 0;
		int certain = 0;
		int maybe = 0;

		get_certains_maybes(total, certain, maybe);
//		std::cout << total << " bonds of which " << certain << " are "\
//		"certainly " << Request << " and " << maybe << " maybe" << std::endl;
		
		/* firstly we impose the range of certain and maybe bonds to sum */
		std::vector<int> possibilities;
		possibilities.reserve(maybe + 1);

		for (int i = certain; i <= certain + maybe; i++)
		{
			possibilities.push_back(i);
		}

		Count::Values count = values_as_count(possibilities);
		_sum.assign_value(count, this, previous);
		
		/* now we find all the possible values of sum, in integer form. */
		std::vector<int> sum_options = possible_values(_sum.value());
		
		/* how many of these are acceptable? */
		std::vector<int> acceptables;
		for (const int &option : sum_options)
		{
			if (option >= certain && option <= certain + maybe)
			{
				acceptables.push_back(option);
			}
		}
		
		/* if there's only one acceptable value and it's either certain or
		 * (certain + maybe) then we can assign the remainder */
		
		if (acceptables.size() == 1)
		{
			if (acceptables[0] == certain)
			{
				Bond::Values not_request;
				not_request = (Bond::Values)(Bond::Unassigned & ~Request);
				tell_maybe_bonds(not_request, previous);
			}
			else if (acceptables[0] == certain + maybe)
			{
				tell_maybe_bonds((Bond::Values)Request, previous);
			}
		}
		/* but if there's no acceptable solution then it's a contradiction */
		else if (acceptables.size() == 0)
		{
			return false;
		}
		
		return true;
	}

	std::vector<BondConnector *> _bonds;
	CountConnector &_sum;
};

typedef BondAdder<Bond::Strong> StrongAdder;
typedef BondAdder<Bond::Weak> WeakAdder;
typedef BondAdder<Bond::Present> PresentAdder;
typedef BondAdder<Bond::Absent> AbsentAdder;
typedef BondAdder<Bond::NotBroken> NotBrokenAdder;
};

#endif
