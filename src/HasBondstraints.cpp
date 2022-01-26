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

#include "HasBondstraints.h"
#include "BondTorsion.h"
#include "BondLength.h"
#include "BondAngle.h"

HasBondstraints::~HasBondstraints()
{

}

bool HasBondstraints::hasBondAngle(BondAngle *angle)
{
	for (size_t i = 0; i < _bondAngles.size(); i++)
	{
		if (*_bondAngles[i] == *angle)
		{
			return true;
		}
	}

	return false;
}
bool HasBondstraints::hasTorsion(BondTorsion *torsion)
{
	for (size_t i = 0; i < _torsions.size(); i++)
	{
		if (*_torsions[i] == *torsion)
		{
			return true;
		}
	}

	return false;
}


bool HasBondstraints::hasBondLength(BondLength *length)
{
	for (size_t i = 0; i < _bondLengths.size(); i++)
	{
		if (*_bondLengths[i] == *length)
		{
			return true;
		}
	}

	return false;
}

void HasBondstraints::addBondstraint(BondLength *length)
{
	if (hasBondLength(length))
	{
		return;
	}

	_bondstraints.push_back(length);
	_bondLengths.push_back(length);
}

void HasBondstraints::addBondstraint(BondAngle *angle)
{
	if (hasBondAngle(angle))
	{
		return;
	}

	_bondstraints.push_back(angle);
	_bondAngles.push_back(angle);
	
	Atom *me = atomIdentity();
	
	if (angle->atomIsCentral(me))
	{
		_centralBondAngles.push_back(angle);
	}
	
	if (angle->atomIsTerminal(me))
	{
		_terminalBondAngles.push_back(angle);
	}
}

void HasBondstraints::addBondstraint(BondTorsion *torsion)
{
	if (hasTorsion(torsion))
	{
		return;
	}

	_bondstraints.push_back(torsion);
	_torsions.push_back(torsion);
	
	Atom *me = atomIdentity();
	
	if (torsion->atomIsCentral(me))
	{
		_centralTorsions.push_back(torsion);
	}
	
	if (torsion->atomIsTerminal(me))
	{
		_terminalTorsions.push_back(torsion);
	}

}

void HasBondstraints::deleteBondstraints()
{
	for (size_t i = 0; i < _bondstraints.size(); i++)
	{
		delete _bondstraints[i];
		_bondstraints[i] = nullptr;
	}
	
	_bondstraints.clear();
}

BondAngle *HasBondstraints::findBondAngle(Atom *left, Atom *centre, Atom *right)
{
	BondAngle ba(nullptr, left, centre, right, 0);

	for (size_t i = 0; i < _bondAngles.size(); i++)
	{
		if (*_bondAngles[i] == ba)
		{
			return _bondAngles[i];
		}
	}
	
	return nullptr;
}

BondLength *HasBondstraints::findBondLength(Atom *a, Atom *b)
{
	BondLength bl(nullptr, a, b, 0);

	for (size_t i = 0; i < _bondLengths.size(); i++)
	{
		if (*_bondLengths[i] == bl)
		{
			return _bondLengths[i];
		}
	}

	return nullptr;
}
