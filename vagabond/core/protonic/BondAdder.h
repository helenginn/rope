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

enum CountType
{
	Certain,
	Maybe,
	Not,
};

inline std::ostream &operator<<(std::ostream &ss, const CountType &ct)
{
	switch (ct)
	{
		case Certain:
		ss << "Certain"; break;
		case Maybe:
		ss << "Maybe"; break;
		case Not:
		ss << "Not"; break;
		default:
		ss << "WeirdCountType"; break;
	}
	return ss;
}

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
	
	CountType type_of_bond(BondConnector *const &bond,
	                       ExistenceConnector *const &exist)
	{
		Existence::Values existence = existence_of(exist);
		Bond::Values obj = bond->value();
		
		if (existence == Existence::Absent)
		{
			return Not;
		}
		else if (existence == Existence::Unassigned)
		{
			if (obj & Request)
			{
				return Maybe;
			}
			else
			{
				return Not;
			}
		}
		else
		{
			if (value_is_not_purely_requested(obj))
			{
				return Maybe;
			}
			else if (value_is_requested_and_nothing_else(obj))
			{
				return Certain;
			}
			else
			{
				return Not;
			}
		}
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

			CountType type = type_of_bond(bond, exist);
//			std::cout << "What type of bond is " << *bond << " (requested:"
//			<< Request << ")? -> " << type << std::endl;
			

//			std::cout << "Checking " << Request << " for " << bond->desc() << " (obj_value=" << bond->value() <<
//			", existence=" << existence_of(exist) << ")" << " --> " << type << std::endl;

			if (type == Certain) certain++;
			if (type == Maybe) maybe++;

			total++;
		}
	}
	
	BondAdder(const std::map<BondConnector *, ExistenceConnector *> 
	          &bonds, CountConnector &sum, ExistenceConnector *coord,
	          const std::string &centre)
	: _bonds(bonds), _sum(sum), _coordExist(coord), _centre(centre)
	{
		std::vector<ConnectBase *> list = {&sum, coord};
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
	
	void forget(const GuiltVersion &gv)
	{
		_sum.forget(gv);

		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			it->first->forget(gv);
			if (it->second)
			{
				it->second->forget(gv);
			}
		}
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
	void tell_maybe_bonds(const Bond::Values &tell, MakeAssign &assign,
	                      bool positive)
	{
		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
			ExistenceConnector *const &exist = it->second;

			Bond::Values obj = bond->value();
			Existence::Values existence = existence_of(exist);

			CountType type = type_of_bond(bond, exist);
			if (type != Maybe)
			{
				continue;
			}
			// don't even think about playing with sampling!
			// a bond adder cannot determine occupancy

			/* go on, tell them they're not requested! */
			if (existence == Existence::Present || !positive)
			{
				assign(*bond, tell);
			}
		}
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		
		if (_coordExist && !(_coordExist->value() & Existence::Present))
		{
//			std::cout << "Skipping evaluation of adder for " << _centre << std::endl;
			return true;
		}

		int total = 0;
		int certain = 0;
		int maybe = 0;
		

		get_certains_maybes(total, certain, maybe);
//		std::cout << "BondAdder (" << Request << ") for " << *_coordExist << std::endl;
//		std::cout << _centre << ": " << total << " bonds of which " << certain << " are "\
		"certainly " << Request << " and " << maybe << " maybe" << std::endl;
//		std::cout << "Bonds: ";
		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
//			std::cout << bond->desc() << ", ";
		}
//		std::cout << std::endl;

		
		/* firstly we impose the range of certain and maybe bonds to sum */
		OpSet<int> possibilities;

		for (int i = certain; i <= certain + maybe; i++)
		{
			possibilities.insert(i);
		}

		/*
		auto print = [](const OpSet<int> &options)
		{
			for (const int &o : options)
			{
				std::cout << o << " ";
			}
			std::cout << std::endl;
		};
		*/

		Count::Values count = values_as_count(possibilities);
		assign(_sum, count);
		if (!assign.okay())
		{
			return assign.okay();
		}
		
		/* now we find all the possible values of sum, in integer form. */
		OpSet<int> sum_options = possible_values(_sum.value());
		
		OpSet<int> common = possibilities.common_to_both(sum_options);

		/*
		std::cout << "Possibilities from counting: ";
		print(possibilities);
		std::cout << "Possibilities from prescribed sum: ";
		print(sum_options);
		std::cout << "Common to both: ";
		print(common);
		*/
		
		/* if there's only one acceptable value and it's either certain or
		 * (certain + maybe) then we can assign the remainder */

		
		if (common.size() == 1)
		{
			if (*common.begin() == certain)
			{
//				std::cout << "Common is equal to certain" << std::endl;
				Bond::Values not_request;
				not_request = (Bond::Values)(Bond::Unassigned & ~Request);
				tell_maybe_bonds(not_request, assign, false);
			}
			else if (*common.begin() == certain + maybe)
			{
//				std::cout << "Common is equal to certain + maybe" << std::endl;
				tell_maybe_bonds((Bond::Values)Request, assign, true);
			}
		}
		/* but if there's no acceptable solution then it's a contradiction */
		else if (common.size() == 0)
		{
			return false;
		}
		
		bool result = assign.okay() && !is_contradictory(_sum.value());
		
		return result;
	}

	std::map<BondConnector *, ExistenceConnector *> _bonds;
	CountConnector &_sum;
	ExistenceConnector *_coordExist{};
	std::string _centre{};
};

typedef BondAdder<Bond::Strong> StrongAdder;
typedef BondAdder<Bond::Weak> WeakAdder;
typedef BondAdder<Bond::Bonded> BondedAdder;
typedef BondAdder<Bond::LonePair> LonePairAdder;
typedef BondAdder<Bond::NotBroken> NotBrokenAdder;
};

#endif
