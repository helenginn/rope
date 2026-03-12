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
	Existence::Values existence_of(ExistenceConnector *const &exist)
	{
		Existence::Values existence = Existence::Present;
		if (exist)
		{
			existence = exist->value();
		}
		return existence;
	}
	
	bool value_is_certain(BondConnector *const &bond,
	                      ExistenceConnector *const &exist)
	{
		Existence::Values existence = existence_of(exist);
		Bond::Values obj = bond->value();

		return (value_is_requested_and_nothing_else(obj) && 
		        (is_definitely_present(existence) || does_not_matter()));
	}

	bool value_is_maybe(BondConnector *const &bond,
	                    ExistenceConnector *const &exist)
	{
		Existence::Values existence = existence_of(exist);
		Bond::Values obj = bond->value();

		return (value_is_not_purely_requested(obj) ||
		        (value_is_requested_and_nothing_else(obj) && 
		        (can_be_present_or_absent(existence) || 
		        does_not_matter())));
	}

	void get_certains_maybes(int &total, int &certain, int &maybe)
	{
		total = 0;
		certain = 0;
		maybe = 0;

		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
			ExistenceConnector *const &exist = it->second;

			if (value_is_certain(bond, exist))
			{
				certain++;
			}
			else if (value_is_maybe(bond, exist))
			{
				maybe++;
			}

			total++;
		}
	}
	
	BondAdder(const std::map<BondConnector *, ExistenceConnector *> 
	          &bonds, CountConnector &sum, ExistenceConnector *coord,
	          const std::string &centre)
	: _bonds(bonds), _sum(sum), _coordExist(coord), _centre(centre)
	{
		std::vector<ConnectBase *> list = {&sum};
		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			list.push_back(it->first);
			if (it->second)
			{
				list.push_back(it->second);
			}
		}
		
		prep_constraints_and_forgets(this, list);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "count of all bonds of type " << Request << 
		" should equal expected sum for " << _sum;
		return ss.str();

	}
	
	void forget(OpSet<void *> &blame)
	{
		_sum.forget(blame);

		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			it->first->forget(blame);
			if (it->second)
			{
				it->second->forget(blame);
			}
		}
	}

	bool does_not_matter()
	{
		return (Request == Bond::NotBroken);
	}
	
	bool can_be_present_or_absent(const Existence::Values &exist)
	{
		return (exist == Existence::Unassigned); 
	}
	
	bool is_definitely_present(const Existence::Values &exist)
	{
		return (exist == Existence::Present); 
	}
	
	bool value_is_not_purely_requested(const Bond::Values &val)
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
	
	template <class MakeAssign>
	void tell_maybe_bonds(const Bond::Values &tell, MakeAssign &assign)
	{
		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
			ExistenceConnector *const &exist = it->second;

			Bond::Values obj = bond->value();
			Existence::Values existence = existence_of(exist);

			bool maybe = (value_is_not_purely_requested(obj) &&
			              (is_definitely_present(existence) || 
			              does_not_matter()));

			/* go on, tell them they're not requested! */
			if (maybe)
			{
				assign(*bond, tell);
			}
		}
	}
	
	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);
		
		if (_coordExist && !(_coordExist->value() == Existence::Present))
		{
//			std::cout << "Skipping evaluation of adder for " << _centre << std::endl;
			return true;
		}

		int total = 0;
		int certain = 0;
		int maybe = 0;

		get_certains_maybes(total, certain, maybe);
//		std::cout << _centre << ": " << total << " bonds of which " << certain << " are "\
//		"certainly " << Request << " and " << maybe << " maybe" << std::endl;
		
		/* firstly we impose the range of certain and maybe bonds to sum */
		std::vector<int> possibilities;
		possibilities.reserve(maybe + 1);

		auto print = [](const std::vector<int> &options)
		{
			return;
			for (const int &o : options)
			{
				std::cout << o << " ";
			}
			std::cout << std::endl;
		};

		for (int i = certain; i <= certain + maybe; i++)
		{
			possibilities.push_back(i);
		}

		print(possibilities);
		Count::Values count = values_as_count(possibilities);
		assign(_sum, count);
		
		/* now we find all the possible values of sum, in integer form. */
		std::vector<int> sum_options = possible_values(_sum.value());
		print(sum_options);
		
		/* how many of these are acceptable? */
		std::vector<int> acceptables;
		for (const int &option : sum_options)
		{
			if (option >= certain && option <= certain + maybe)
			{
				acceptables.push_back(option);
			}
		}
		
		print(sum_options);
		/* if there's only one acceptable value and it's either certain or
		 * (certain + maybe) then we can assign the remainder */
		
		if (acceptables.size() == 1)
		{
			if (acceptables[0] == certain)
			{
				Bond::Values not_request;
				not_request = (Bond::Values)(Bond::Unassigned & ~Request);
				tell_maybe_bonds(not_request, assign);
			}
			else if (acceptables[0] == certain + maybe)
			{
				tell_maybe_bonds((Bond::Values)Request, assign);
			}
		}
		/* but if there's no acceptable solution then it's a contradiction */
		else if (acceptables.size() == 0)
		{
			return false;
		}
		
		return assign.okay();
	}

	std::map<BondConnector *, ExistenceConnector *> _bonds;
	CountConnector &_sum;
	ExistenceConnector *_coordExist{};
	std::string _centre{};
};

typedef BondAdder<Bond::Strong> StrongAdder;
typedef BondAdder<Bond::Weak> WeakAdder;
typedef BondAdder<Bond::Present> PresentAdder;
typedef BondAdder<Bond::Absent> AbsentAdder;
typedef BondAdder<Bond::NotBroken> NotBrokenAdder;
};

#endif
