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
#include "ConstraintBase.h"
#include "Attachment.h"

namespace hnet
{
/* addition of all strong bonds from an atom and output of a total sum */
template <Bond::Values Request>
struct BondAdder : public ConstraintBase
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
	
	void get_certains_maybes(int &total, int &certain, int &maybe)
	{
		total = 0;
		certain = 0;
		maybe = 0;

		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
			ExistenceConnector *const &exist = it->second;

			Attachment attach(*bond, *exist);
			CountType type = attach.type_of_bond(Request);
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
	                      CountType certainty)
	{
		for (auto it = _bonds.begin(); it != _bonds.end(); it++)
		{
			BondConnector *const &bond = it->first;
			ExistenceConnector *const &exist = it->second;

			Attachment attach(*bond, *exist);
			CountType type = attach.type_of_bond(Request);

			if (type != Maybe)
			{
				continue;
			}
			if (_coordExist->value() & Existence::Absent)
			{
				certainty = Maybe;
			}

			attach.inform(tell, assign, certainty);

			// don't even think about playing with sampling!
			// a bond adder cannot determine occupancy
			// 			^ update: LIES?

			/* go on, tell them they're not requested! */
//			if (existence == Existence::Present || !positive)
			{
//				assign(*bond, tell);
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
//		"certainly " << Request << " and " << maybe << " maybe" << std::endl;
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
		/*
		assign(_sum, count);
		if (!assign.okay())
		{
			return assign.okay();
		}
		*/
		
		/* now we find all the possible values of sum, in integer form. */
		OpSet<int> sum_options = possible_values(_sum.value());
		
		OpSet<int> common = possibilities.common_to_both(sum_options);
		
		/* if there's only one acceptable value and it's either certain or
		 * (certain + maybe) then we can assign the remainder */

		if (common.size() >= 1)
		{
			assign(_sum, count);
		}
		if (common.size() == 1)
		{
			if (*common.begin() == certain)
			{
				tell_maybe_bonds((Bond::Values)~Request, assign, Certain);
			}
			else if (*common.begin() == certain + maybe)
			{
				tell_maybe_bonds((Bond::Values)Request, assign, Certain);
			}
		}
		/* if we're about to kill off a bond count, but we could set the
		 * atom existence instead, do that */
		else if (common.size() == 0 && (_coordExist->value() & Existence::Absent))
		{
			assign(*_coordExist, Existence::Absent);
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
