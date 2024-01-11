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
	HydrogenBond(BondConnector &left, HydrogenConnector &centre, 
	             BondConnector &right) 
	: _left(left), _centre(centre), _right(right)
	{
		auto self_check = [this]() { return check(); };

		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);
		_centre.add_constraint_check(self_check);
		
		if (!check())
		{
			_left.pop_last_check();
			_centre.pop_last_check();
			_right.pop_last_check();

			throw std::runtime_error("New hydrogen bond immediately "\
			                         "failed validation check");
		}
	}
	
	bool check_conditions()
	{
		if ((_left.value() & Bond::Strong) && 
		    (_centre.value() & Hydrogen::Present) && 
		    (_right.value() & Bond::Weak))
		{
			return true;
		}

		if ((_left.value() & Bond::Weak) && 
		    (_centre.value() & Hydrogen::Present) && 
		    (_right.value() & Bond::Strong))
		{
			return true;
		}

		if ((_left.value() & Bond::Absent) && 
		    (_centre.value() & Hydrogen::Absent) && 
		    (_right.value() & Bond::Absent))
		{
			return true;
		}
		
		return false;
	}
	
	bool bond_definitely_present(const Bond::Values &val)
	{
		return (val & Bond::Present) && !(val & Bond::Absent);
	}
	
	bool impose()
	{
		Bond::Values forLeft = _left.value();
		Hydrogen::Values forCentre = _centre.value();
		Bond::Values forRight = _right.value();

		if (_centre.value() == Hydrogen::Absent)
		{
			/* if hydrogen is absent, bonds to it must also be absent */
			forLeft = Bond::Values(forLeft & Bond::Absent);
			forRight = Bond::Values(forRight & Bond::Absent);
		}
		else if (_centre.value() == Hydrogen::Present)
		{
			/* if hydrogen is absent, no bond to it may be absent */
			forLeft = Bond::Values(forLeft & Bond::Present);
			forRight = Bond::Values(forRight & Bond::Present);
		}
		
		/* if there bond is present, hydrogen must exist */
		if (bond_definitely_present(_left.value()) ||
		    bond_definitely_present(_right.value()))
		{
			forCentre = Hydrogen::Values(forCentre & Hydrogen::Present);
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
		else if (_right.value() == Bond::Strong)
		{
			forLeft = Bond::Values(forLeft & Bond::Weak);
		}
		else if (_right.value() == Bond::Weak)
		{
			forLeft = Bond::Values(forLeft & Bond::Strong);
		}
		
		bool check_left = _left.assign_value_without_checking(forLeft, this);
		bool check_centre = _centre.assign_value_without_checking(forCentre, this);
		bool check_right = _right.assign_value_without_checking(forRight, this);

		if (check_left && !_left.check_all()) return false;
		if (check_centre && !_centre.check_all()) return false;
		if (check_right && !_right.check_all()) return false;

		return true;
	}
	
	bool check()
	{
		bool result = check_conditions();
		if (result)
		{
			return impose();
		}

		return false;
	}
	
	BondConnector &_left;
	HydrogenConnector &_centre;
	BondConnector &_right;
};
};


#endif
