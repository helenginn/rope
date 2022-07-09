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
#include "matrix_functions.h"
#include "BondTorsion.h"
#include "AtomGroup.h"
#include "Atom.h"

double BondTorsion::_maxSeparation = 20;

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
	
	if (_a == nullptr || _b == nullptr || _c == nullptr || _d == nullptr)
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

bool BondTorsion::coversMainChain()
{
	return ((_b->isMainChain() && _c->isMainChain() && _d->isMainChain()) ||
	        (_a->isMainChain() && _b->isMainChain() && _c->isMainChain()));
}

bool BondTorsion::isConstrained() const
{
	return _constrained;

}

const std::string BondTorsion::reverse_desc() const
{
	return _d->atomName() + "-" + _c->atomName() + "-" + 
	_b->atomName() + "-" + _a->atomName();
}

const std::string BondTorsion::desc() const
{
	return _a->atomName() + "-" + _b->atomName() + "-" + 
	_c->atomName() + "-" + _d->atomName();
}

double BondTorsion::startingAngle()
{
	if (isConstrained())
	{
		return _angle;
	}
	
	if (_refined)
	{
		return _refinedAngle;
	}

	return measurement(SourceInitial);
}

double BondTorsion::measurement(BondTorsion::Source source)
{
	if (_measured && source == SourceInitial)
	{
		return _measuredAngle;
	}

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
	
	double putative = measure_bond_torsion(poz);
	
	if (putative != putative)
	{
		putative = 0;
	}
	
	_measured = true;
	_measuredAngle = putative;
	
	return putative;
}

double BondTorsion::similarityScore(BondTorsion *const other) const
{
	glm::vec3 x = glm::vec3(1., 0., 0.);
	glm::vec3 average = bondDirection() + other->bondDirection();
	average = normalize(average);

	glm::mat3x3 twizzle = glm::inverse(mat3x3_rhbasis(average, x));
	
	if (!is_glm_vec_sane(twizzle[0]))
	{
		x = glm::vec3(0., 1., 0.);
		twizzle = glm::transpose(mat3x3_rhbasis(average, x));
	}

	double dot = 0;
	double dist = 0;

	{
		glm::vec3 mine = twizzle * bondDirection();
		glm::vec3 yours = twizzle * other->bondDirection();

		glm::vec2 my2D = glm::normalize(glm::vec2(mine.x, mine.z));
		glm::vec2 your2D = glm::normalize(glm::vec2(yours.x, yours.z));

		dot = glm::dot(my2D, your2D);
	}

	{
		glm::vec3 mine = twizzle * bondMidPoint();
		glm::vec3 yours = twizzle * other->bondMidPoint();
		mine.x = 0;
		yours.x = 0;
		

		dist = glm::length(mine - yours);
	}
	
	double dist_mult = (_maxSeparation - dist) / _maxSeparation;
	if (dist_mult < 0) dist_mult = 0;
	
	return dot * dist_mult;
}

glm::vec3 BondTorsion::bondDirection() const
{
	glm::vec3 diff = atom(2)->derivedPosition();
	diff -= atom(1)->derivedPosition();

	return diff;
}

glm::vec3 BondTorsion::bondMidPoint() const
{
	glm::vec3 sum = atom(1)->derivedPosition();
	sum += atom(2)->derivedPosition();

	sum /= 2;
	return sum;
}

const ResidueId BondTorsion::residueId() 
{
	if (_gotId)
	{
		return _resId;
	}

	if (_b->atomName() < _c->atomName())
	{
		_resId = _b->residueId();
	}
	else
	{
		_resId = _c->residueId();
	}

	return _resId;
}

