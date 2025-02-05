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

#include "BondLength.h"
#include "AtomGroup.h"

BondLength::BondLength(AtomGroup *owner, Atom *a, Atom *b, 
                       double length, double stdev)
{
	_owner = owner;
	_a = a;
	_b = b;
	_length = length;
	_stdev = stdev;
//	_stdev += 0.01;

	if (_a == nullptr || _b == nullptr)
	{
		throw(std::runtime_error("Initialising bond length with null values"));
	}
	
	if (owner && (!owner->hasAtom(a) || !owner->hasAtom(b)))
	{
		throw(std::runtime_error("Owner does not own atom assigned to BondLength"));
	}
	
	if (_owner)
	{
		_a->addBondstraint(this);
		_b->addBondstraint(this);
		_owner->addBondstraint(this);
	}
}

bool BondLength::operator==(const BondLength &other) const
{
	if ((_a == other._a && _b == other._b) || (_a == other._b && _b == other._a))
	{
		return true;
	}

	return false;
}

Atom *BondLength::otherAtom(const Atom *a) const
{
	if (_a == a)
	{
		return _b;
	}
	else if (_b == a)
	{
		return _a;
	}

	throw(std::runtime_error("Queried atom not either atom in bond length"));
}

const double BondLength::measurement() const
{
	glm::vec3 bond = _a->derivedPosition() - _b->derivedPosition();
	float length = glm::length(bond);

	return length;
}

const std::string BondLength::desc()
{
	std::string desc = _a->atomName() + "-" + _b->atomName();
	return desc;
}
