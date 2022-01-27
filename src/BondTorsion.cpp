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

#include <iostream>
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
	
	if (_a == NULL || _b == NULL || _c == NULL || _d == NULL)
	{
		throw(std::runtime_error("Initialising bond torsion with null values"));
	}
	
	if (owner && (!owner->hasAtom(a) || !owner->hasAtom(b) 
	    || !owner->hasAtom(c) || !owner->hasAtom(d)))
	{
		throw(std::runtime_error("Owner does not own atom "
		                         "assigned to BondTorsion"));
	}
	
	if (_owner)
	{
		_a->addBondstraint(this);
		_b->addBondstraint(this);
		_c->addBondstraint(this);
		_d->addBondstraint(this);
		_owner->addBondstraint(this);
	}
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

bool BondTorsion::isConstrained() const
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

double BondTorsion::startingAngle() const
{
	if (isConstrained() && _angle >= 0)
	{
		return _angle;
	}

	return measurement(SourceInitial);
}

double BondTorsion::measurement(BondTorsion::Source source) const
{
	glm::vec3 poz[4];
	
	for (size_t i = 0; i < 4; i++)
	{
		if (source == SourceInitial)
		{
			poz[i] = atom(i)->initialPosition();
		}
		else if (source == SourceDerived)
		{
			poz[i] = atom(i)->derivedPosition();
		}
	}
	
	glm::mat3x3 squish;
	squish[0] = poz[0] - poz[1];
	squish[2] = poz[2] - poz[1];
	squish[1] = glm::cross(squish[2], squish[0]);
	
	glm::mat3x3 inv = glm::inverse(squish);

	glm::vec3 q = glm::normalize(inv * (poz[3] - poz[2]));
	glm::vec3 axis = glm::vec3(1., 0., 0.);
	
	double angle = glm::angle(axis, q);

	return rad2deg(angle);
}
