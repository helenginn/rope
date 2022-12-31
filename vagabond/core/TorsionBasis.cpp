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
#include "TorsionBasis.h"
#include "BondTorsion.h"
#include "SimpleBasis.h"
#include "ConcertedBasis.h"
#include "engine/MechanicalBasis.h"

TorsionBasis::TorsionBasis()
{

}

TorsionBasis *TorsionBasis::newBasis(Type type)
{
	TorsionBasis *basis = nullptr;

	if (type == TorsionBasis::TypeSimple)
	{
		basis = new SimpleBasis();
	}
	else if (type == TorsionBasis::TypeCustom)
	{
		ConcertedBasis *cb = new ConcertedBasis();
		cb->setCustom(true);
		basis = cb;
	}
	else if (type == TorsionBasis::TypeConcerted)
	{
		basis = new ConcertedBasis();
	}
	else
	{
		basis = new MechanicalBasis();
	}

	return basis;
}

void TorsionBasis::absorbVector(const float *vec, int n, bool *mask)
{
	for (size_t i = 0; i < torsionCount(); i++)
	{
		if (mask && !mask[i])
		{
			continue;
		}

		float torsion = torsionForVector(i, vec, n);
		_torsions[i]->setRefinedAngle(torsion);
		_angles[i].angle = torsion;
	}

}

int TorsionBasis::addTorsion(BondTorsion *torsion, Atom *atom)
{
	if (torsion == nullptr || torsion->isConstrained())
	{
		return -1;
	}
	
	for (size_t i = 0; i < _torsions.size(); i++)
	{
		if (_torsions[i] == torsion)
		{
			return i;
		}
	}

	_torsions.push_back(torsion);
	_atoms.push_back(atom);

	return _torsions.size() - 1;
}

void TorsionBasis::prepare(int dims)
{

}

int TorsionBasis::indexForTorsion(BondTorsion *bt)
{
	int i = 0;
	for (BondTorsion *torsion : _torsions)
	{
		if (bt == torsion)
		{
			return i;
		}
		i++;
	}
	return -1;
}
