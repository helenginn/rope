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


#ifndef __vagabond__Constraint__
#define __vagabond__Constraint__

#include <variant>
#include <functional>
#include "Connector.h"

namespace hnet
{
/* simple constant class to impose a value (AND) on a connector */
template <class Connector, class Value>
struct Constant
{
	Constant(Connector &connector, const Value &constant) :
	_connector(connector), _constant(constant)
	{
		/* add this object's constraint check function to the connector */
		_connector.add_constraint_check([this](){ return check(); });

		if (!check())
		{
			throw std::runtime_error("New constraint immediately "\
			                         "failed validation check");
		}
	}
	
	bool check()
	{
		Value candidate = Value(_connector.value() & _constant);
		if (is_contradictory(candidate))
		{
			return false;
		}
		else
		{
			return _connector.assign_value(_constant);
		}
	}
	
	Connector &_connector;
	Value _constant;
};

/* cosmetic link where there are no constraints, but useful to conceptually
 * link for display */
template <class LeftConnector, class RightConnector>
struct Cosmetic
{
	Cosmetic(LeftConnector &left, RightConnector &right) 
	: _left(left), _right(right)
	{
		auto self_check = [this]() { return check(); };

		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);
	}

	bool check()
	{
		return true;
	}

	LeftConnector &_left;
	RightConnector &_right;
};


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
		if (((_left.value() & Bond::Present) != Bond::Contradiction)
		    || ((_right.value() & Bond::Present) != Bond::Contradiction))
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
		
		bool check_left = _left.assign_value_without_checking(forLeft);
		bool check_centre = _centre.assign_value_without_checking(forCentre);
		bool check_right = _right.assign_value_without_checking(forRight);

		if (check_left && !_left.check_all()) return false;
		if (check_centre && !_centre.check_all()) return false;
		if (check_right && !_right.check_all()) return false;
		
		std::cout << _left.value() << " " << _centre.value() << " " 
		<< _right.value() << std::endl;

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

typedef Constant<AtomConnector, Atom::Values> AtomConstant;
typedef Cosmetic<AtomConnector, BondConnector> AtomToBond;

/* union to store created constraints in a list */
struct AnyConstraint
{
	enum Type
	{
		Atom, AtomBond, HBond
	};
	
	AnyConstraint(AtomConstant *const &constraint)
	{
		_type = Atom;
		_ptr = constraint;
	}
	
	AnyConstraint(AtomToBond *const &constraint)
	{
		_type = AtomBond;
		_ptr = constraint;
	}
	
	AnyConstraint(HydrogenBond *const &constraint)
	{
		_type = HBond;
		_ptr = constraint;
	}
	
	void destroy()
	{
		switch (_type)
		{
			case Atom:
			delete static_cast<AtomToBond *>(_ptr);
			break;

			case AtomBond:
			delete static_cast<AtomConstant *>(_ptr);
			break;

			case HBond:
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
