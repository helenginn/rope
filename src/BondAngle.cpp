// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "BondAngle.h"
#include "AtomGroup.h"
#include "Atom.h"

BondAngle::BondAngle(AtomGroup *owner, Atom *a, Atom *b, Atom *c, double angle)
{
	_owner = owner;
	_a = a;
	_b = b;
	_c = c;
	_angle = angle;
	
	if (_a == NULL || _b == NULL || _c == NULL)
	{
		throw(std::runtime_error("Initialising bond angle with null values"));
	}
	
	if (owner && (!owner->hasAtom(a) || !owner->hasAtom(b) 
	              || !owner->hasAtom(c)))
	{
		throw(std::runtime_error("Owner does not own atom assigned to BondAngle"));
	}
	
	if (_owner)
	{
		_a->addBondstraint(this);
		_b->addBondstraint(this);
		_c->addBondstraint(this);
		_owner->addBondstraint(this);
	}
}

bool BondAngle::operator==(const BondAngle &other) const
{
	if ((_a == other._a && _b == other._b && _c == other._c) 
	    || (_a == other._c && _b == other._b && _c == other._a))
	{
		return true;
	}

	return false;
}


bool BondAngle::formsTorsionWith(BondAngle *o)
{
	if (_b == o->_a && _c == o->_b && _a != o->_c)
	{
		return true;
	}
	else if (_b == o->_c && _c == o->_b && _a != o->_a)
	{
		return true;
	}
	else if (_b == o->_a && _a == o->_b && _c != o->_c)
	{
		return true;
	}
	else if (_b == o->_c && _a == o->_b && _c != o->_a)
	{
		return true;
	}

	return false;
}


void BondAngle::getSequentialAtoms(BondAngle *o, Atom **a, Atom **b, 
                                   Atom **c, Atom **d)
{
	if (_b == o->_a && _c == o->_b)
	{
		*a = _a;
		*b = _b;
		*c = o->_b;
		*d = o->_c;
	}
	else if (_b == o->_c && _c == o->_b)
	{
		*a = _a;
		*b = _b;
		*c = o->_b;
		*d = o->_a;
	}
	else if (_b == o->_a && _a == o->_b)
	{
		*a = _c;
		*b = _b;
		*c = o->_b;
		*d = o->_c;
	}
	else if (_b == o->_c && _a == o->_b)
	{
		*a = _c;
		*b = _b;
		*c = o->_b;
		*d = o->_a;
	}
}

std::string BondAngle::desc()
{
	return _a->atomName() + "-" + _b->atomName() + "-" + _c->atomName();
}
