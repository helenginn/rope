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
struct HydrogenBond
{
	HydrogenBond(BondConnector &left, ExistenceConnector &centre, 
	             BondConnector &right) 
	: _left(left), _centre(centre), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &centre, &right});
	}
	
	void forget(OpSet<void *> &blame)
	{
		_left.forget(blame);
		_centre.forget(blame);
		_right.forget(blame);
	}
	
	bool bond_definitely_present(const Bond::Values &val)
	{
		return (val & Bond::Present) && !(val & Bond::NotPresent);
	}
	
	bool bond_definitely_not_used(const Bond::Values &val)
	{
		return (val == Bond::Absent || val == Bond::Broken || 
		        val == Bond::NotPresent);
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
	
	bool impose(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);
		Bond::Values forLeft = _left.value();
		Existence::Values forCentre = _centre.value();
		Bond::Values forRight = _right.value();

		if ((_centre.value() == Existence::Absent) ||
		    bond_definitely_not_used(_left.value()) ||
		    bond_definitely_not_used(_right.value()))
		{
			/* if anything is absent, hydrogen & all bonds must also be absent */
			forLeft = Bond::Values(forLeft & Bond::NotPresent);
			forCentre = Existence::Values(forCentre & Existence::Absent);
			forRight = Bond::Values(forRight & Bond::NotPresent);
		}

		if ((_centre.value() == Existence::Present) ||
		    bond_definitely_present(_left.value()) ||
		    bond_definitely_present(_right.value()))
		{
			/* if anything is present, hydrogen & all bonds must also be present */
			forLeft = Bond::Values(forLeft & Bond::Present);
			forRight = Bond::Values(forRight & Bond::Present);
			forCentre = Existence::Values(forCentre & Existence::Present);
		}

		/* must also impose the strong/weak bond dichotomy, 
		 * i.e.: X----H . . . Y has strong X-H and weak H-Y bond */

		if (_left.value() == Bond::Strong)
		{
			forRight = Bond::Values(forRight & Bond::Weak);
		}
		else if (_left.value() == Bond::Weak)
		{
			forRight = Bond::Values(forRight & Bond::Strong);
		}

		if (_right.value() == Bond::Strong)
		{
			forLeft = Bond::Values(forLeft & Bond::Weak);
		}
		else if (_right.value() == Bond::Weak)
		{
			forLeft = Bond::Values(forLeft & Bond::Strong);
		}
		
		// if each bond on either side can only do weak, we cannot have a
		// hydrogen bond
		if (_right.value() & Bond::Weak 
		    && _left.value() & Bond::Weak
		    && !(_right.value() & Bond::Strong) && 
		       !(_left.value() & Bond::Strong))
		{
			forCentre = Existence::Values(forCentre & Existence::Absent);
		}
		
		assign(_left, forLeft);
		assign(_centre, forCentre);
		assign(_right, forRight);

		return assign.okay();
	}
	
	bool check(void *previous)
	{
		bool result = impose(previous);

		return result;
	}
	
	BondConnector &_left;
	ExistenceConnector &_centre;
	BondConnector &_right;
};
};


#endif
