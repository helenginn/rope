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
#include "HasBondstraints.h"
#include "BondTorsion.h"
#include "HyperValue.h"
#include "BondLength.h"
#include "BondAngle.h"
#include "Chirality.h"
#include "Atom.h"
#include "AtomGroup.h"

HasBondstraints::~HasBondstraints()
{

}

HasBondstraints::HasBondstraints()
{
	_owns = false;
}

HasBondstraints::HasBondstraints(const HasBondstraints &other)
{
	_owns = false;
	_bondstraints = other._bondstraints;
	_bondLengths = other._bondLengths;
	_bondAngles = other._bondAngles;
	_centralBondAngles = other._centralBondAngles;
	_terminalBondAngles = other._terminalBondAngles;
	_torsions = other._torsions;
	_hyperValues = other._hyperValues;
	_terminalTorsions = other._terminalTorsions;
	_centralTorsions = other._centralTorsions;
	_chirals = other._chirals;

}

bool HasBondstraints::hasHyperValue(HyperValue *hv)
{
	return (_hyperValueMap.count(hv->key(0)));
}

bool HasBondstraints::hasBondAngle(BondAngle *angle)
{
	return (_angleMap.count(angle->key(0)));
}

bool HasBondstraints::hasTorsion(BondTorsion *torsion)
{
	return (_torsionMap.count(torsion->key(0)));
}

bool HasBondstraints::hasChirality(Chirality *chir)
{
	return (_chiralMap.count(chir->key(0)));
}

bool HasBondstraints::hasBondLength(BondLength *length)
{
	return (_lengthMap.count(length->key(0)));
}

void HasBondstraints::addBondstraint(BondLength *length)
{
	if (hasBondLength(length))
	{
		return;
	}

	_bondstraints.push_back(length);
	_bondLengths.push_back(length);
	
	for (size_t i = 0; i < length->keyCount(); i++)
	{
		_lengthMap[length->key(i)] = length;
	}
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

	for (size_t i = 0; i < angle->keyCount(); i++)
	{
		_angleMap[angle->key(i)] = angle;
	}
}

void HasBondstraints::addBondstraint(HyperValue *hv)
{
	if (hasHyperValue(hv))
	{
		return;
	}
	
	_bondstraints.push_back(hv);
	_hyperValues.push_back(hv);

	for (size_t i = 0; i < hv->keyCount(); i++)
	{
		_hyperValueMap[hv->key(i)] = hv;
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
	
	for (size_t i = 0; i < torsion->keyCount(); i++)
	{
		_torsionMap[torsion->key(i)] = torsion;
	}
	
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

void HasBondstraints::addBondstraint(Chirality *chir)
{
	if (hasChirality(chir))
	{
		return;
	}

	_bondstraints.push_back(chir);
	_chirals.push_back(chir);
	
	Atom *me = atomIdentity();

	if (me && chir->centreAtom() != me)
	{
		throw(std::runtime_error("Adding chirality to inappropriate "
		                         "centre atom"));
	}

	for (size_t i = 0; i < chir->keyCount(); i++)
	{
		_chiralMap[chir->key(i)] = chir;
	}
}

void HasBondstraints::deleteBondstraints()
{
	if (_owns)
	{
		for (size_t i = 0; i < _bondstraints.size(); i++)
		{
			delete _bondstraints[i];
		}

		_bondstraints.clear();
	}
}

HyperValue *HasBondstraints::findHyperValue(Atom *atom)
{
	Bondstraint::Key key = Bondstraint::Key(atom);
	if (_hyperValueMap.count(key))
	{
		HyperValue *hv = static_cast<HyperValue *>(_hyperValueMap[key]);
		return hv;
	}
	
	return nullptr;
}

BondAngle *HasBondstraints::findBondAngle(Atom *left, Atom *centre, Atom *right,
                                          bool throw_on_failure)
{
	BondAngle ba(nullptr, left, centre, right, 0);

	for (size_t i = 0; i < _bondAngles.size(); i++)
	{
		if (*_bondAngles[i] == ba)
		{
			return _bondAngles[i];
		}
	}
	
	if (throw_on_failure)
	{
		throw (std::runtime_error("No bond angle " + ba.desc() + " found"
		" between " + left->desc() + " " + centre->desc() + " " +
		right->desc()));
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

BondTorsion *HasBondstraints::findBondTorsion(Atom *a, Atom *b, Atom *c, Atom *d)
{
	BondTorsion bt(nullptr, a, b, c, d, 0);

	for (size_t i = 0; i < _torsions.size(); i++)
	{
		if (*_torsions[i] == bt)
		{
			return _torsions[i];
		}
	}

	return nullptr;
}

Chirality *HasBondstraints::findChirality(Atom *cen, Atom *a, Atom *b, Atom *c)
{
	Chirality bt(nullptr, cen, a, b, c, 0);

	for (size_t i = 0; i < _chirals.size(); i++)
	{
		if (*_chirals[i] == bt)
		{
			return _chirals[i];
		}
	}

	return nullptr;
}

void HasBondstraints::giveBondstraintOwnership(AtomGroup *which)
{
	if (_owns == true && which->_owns == false)
	{
		_owns = false;
		which->_owns = true;

	}
}
