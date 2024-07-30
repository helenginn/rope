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
#include <fstream>

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct HydrogenBond
{
	HydrogenBond(BondConnector &left, HydrogenConnector &centre, 
	             BondConnector &right) 
	: _left(left), _centre(centre), _right(right)
	{
		auto self_check = [this](void *prev) { return check(prev); };

		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);
		_centre.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };

		_left.add_forget(forget_me);
		_centre.add_forget(forget_me);
		_right.add_forget(forget_me);
		
		if (!check(this))
		{
			_left.pop_last_check(this);
			_centre.pop_last_check(this);
			_right.pop_last_check(this);

			throw std::runtime_error("New hydrogen bond immediately "\
			                         "failed validation check");
		}
	}
	
	void forget(void *blame)
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
	
	// void print_bond()
	// {
	// 	// std::ofstream outputFile("hbond_output.txt");

	// 	std::cout << _left.value() << " " << _centre.value() << 
	// 	" " << _right.value() << std::endl;
		
	// 	if (_left.value() == Bond::Contradiction)
	// 	{
	// 		_left.report();
	// 	}
	// 	else if (_right.value() == Bond::Contradiction)
	// 	{
	// 		_right.report();
	// 	}
	// }
	
	void print_bond()
	{
	    std::ofstream outputFile("hbond_output.txt", std::ios_base::app); // Open in append mode

	    if (!outputFile.is_open())
	    {
	        std::cerr << "Failed to open output file." << std::endl;
	        return;
	    }

	    outputFile << _left.value() << " " << _centre.value() << " " << _right.value() << std::endl;

	    if (_left.value() == Bond::Contradiction)
	    {
	        _left.report();
	    }
	    else if (_right.value() == Bond::Contradiction)
	    {
	        _right.report();
	    }

	    outputFile.close(); // Close the file after writing
	}


	bool impose(void *prev)
	{

		Bond::Values forLeft = _left.value();
		Hydrogen::Values forCentre = _centre.value();
		Bond::Values forRight = _right.value();

		if ((_centre.value() == Hydrogen::Absent) ||
		    bond_definitely_not_used(_left.value()) ||
		    bond_definitely_not_used(_right.value()))
		{
			/* if anything is absent, hydrogen & all bonds must also be absent */
			forLeft = Bond::Values(forLeft & Bond::NotPresent);
			forCentre = Hydrogen::Values(forCentre & Hydrogen::Absent);
			forRight = Bond::Values(forRight & Bond::NotPresent);
		}

		if ((_centre.value() == Hydrogen::Present) ||
		    bond_definitely_present(_left.value()) ||
		    bond_definitely_present(_right.value()))
		{
			/* if anything is present, hydrogen & all bonds must also be present */
			forLeft = Bond::Values(forLeft & Bond::Present);
			forRight = Bond::Values(forRight & Bond::Present);
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

		if (_right.value() == Bond::Strong)
		{
			forLeft = Bond::Values(forLeft & Bond::Weak);
		}
		else if (_right.value() == Bond::Weak)
		{
			forLeft = Bond::Values(forLeft & Bond::Strong);
		}
		
		_left.assign_value(forLeft, this, prev);
		if (is_contradictory(_left.value())) return false;

		_centre.assign_value(forCentre, this, prev);
		if (is_contradictory(_centre.value())) return false;

		_right.assign_value(forRight, this, prev);
		if (is_contradictory(_right.value())) return false;

		print_bond();
		return true;
	}
	
	bool check(void *previous)
	{
		bool result = impose(previous);

		return result;
	}
	
	BondConnector &_left;
	HydrogenConnector &_centre;
	BondConnector &_right;
};
};


#endif
