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

#include "BondTorsion.h"
#include "AtomGroup.h"
#include "Atom.h"

BondTorsion::BondTorsion(AtomGroup *owner, Atom *a, Atom *b, Atom *c, 
                         Atom *d, double angle)
{
	_owner = owner;
	_constrained = false;
	_a = a;
	_b = b;
	_c = c;
	_d = d;
	_angle = angle;
	
	if (_owner == NULL || _a == NULL || _b == NULL || _c == NULL)
	{
		throw(std::runtime_error("Initialising bond torsion with null values"));
	}
	
	if (!owner->hasAtom(a) || !owner->hasAtom(b) || !owner->hasAtom(c)
	    || !owner->hasAtom(d))
	{
		throw(std::runtime_error("Owner does not own atom "
		                         "assigned to BondTorsion"));
	}
	
	_a->addBondstraint(this);
	_b->addBondstraint(this);
	_c->addBondstraint(this);
	_d->addBondstraint(this);
	_owner->addBondstraint(this);
}

bool BondTorsion::operator==(const BondTorsion &other) const
{
	if (_a == other._a && _b == other._b && _c == other._c && _d == other._d)
	{
		return true;
	}

	if (_a == other._d && _b == other._c && _c == other._b && _d == other._a)
	{
		return true;
	}

	return false;
}

bool BondTorsion::isConstrained()
{
	if (_a->elementSymbol() == "H" || _b->elementSymbol() == "H" ||
	    _c->elementSymbol() == "H" || _d->elementSymbol() == "H")
	{
		return true;
	}

	return _constrained;

}

const std::string BondTorsion::desc() const
{
	return _a->atomName() + "-" + _b->atomName() + "-" + 
	_c->atomName() + "-" + _d->atomName();
}
