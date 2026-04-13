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

#ifndef __vagabond__HydrogenBond__
#define __vagabond__HydrogenBond__

#include "hnet.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct HydrogenBond : public ConstraintBase
{
	HydrogenBond(BondConnector &left, ExistenceConnector &centre, 
	             BondConnector &right) 
	: _left(left), _centre(centre), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &centre, &right});
	}
	
	bool bond_weak_or_broken(const Bond::Values &val)
	{
		return (val & Bond::Weak) && !(val & Bond::Broken);
	}
	
	bool bond_definitely_present(const Bond::Values &val)
	{
		return (val & Bond::Bonded) && !(val & Bond::NotBonded);
	}
	
	bool bond_definitely_not_bonded(const Bond::Values &val)
	{
		return !(val & Bond::Bonded);
	}
	
	std::string desc()
	{
		return "Hydrogen bonding pattern between \"" + _left.desc() + "\", \"" + 
		_centre.desc() + "\", \"" + _right.desc() + "\"";
	}
	
	void print_bond()
	{
		std::cout << _left.value() << " " << _centre.value() << 
		" " << _right.value() << std::endl;
		
		if (_left.value() == Bond::Contradiction)
		{
			_left.report();
		}
		else if (_right.value() == Bond::Contradiction)
		{
			_right.report();
		}
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		// if H is missing, it can only be a lone pair OR broken bond
		if (_centre.value() == Existence::Absent)
		{
			assign(_left, Bond::NotBonded, "an absent hydrogen cannot be "\
			       "adjacent to a donor or acceptor bond");
		}
		
		// if H is present, one bond must be a donor, other cannot be a donor
		if (_centre.value() == Existence::Present)
		{
			if (_right.value() == Bond::Strong)
			{
				assign(_left, Bond::NotStrong, "a hydrogen with a donor on"\
				       " one side must be accepted on the other");
			}
			
			if (!(_right.value() & Bond::Strong))
			{
				assign(_left, Bond::Strong, "a hydrogen with a non-donor on "\
				       "one side must be a donor on the other");
			}

			if (bond_definitely_not_bonded(_right.value()) &&
			    bond_definitely_present(_left.value()))
			{
				assign(_left, Bond::NotWeak,
				       "if H-bond is not complete on "\
				       "both sides, remaining side cannot be acceptor/broken");
			}
		}

		// if we only have choice between lone pair and acceptor, it's acceptor
		if (_left.value() == Bond::LonePairOrWeak)
		{
			assign(_left, Bond::Weak, "if we only have choice between lone "\
			       "pair and acceptor, it's acceptor");
		}
		
		// if we definitely have a donor/acceptor bond then we must have H
		if (bond_definitely_present(_left.value()))
		{
			assign(_centre, Existence::Present, "a donor/acceptor bond must "\
			       "have a present hydrogen");
		}
		
		if (bond_definitely_not_bonded(_left.value()) && 
		    bond_definitely_not_bonded(_right.value()))
		{
			assign(_centre, Existence::Absent, "a hydrogen braced by two "\
			       "non-bonds must be absent");
		}
		
		if (_right.value() == Bond::Broken)
		{
			assign(_left, Bond::NotWeak, "a broken bond on one side cannot "\
			       "be juxtaposed by an acceptor bond on the other");
		}

		return assign.okay();
	}
	
	BondConnector &_left;
	ExistenceConnector &_centre;
	BondConnector &_right;
};
};


#endif
