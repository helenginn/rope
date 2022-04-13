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
	else
	{
		basis = new ConcertedBasis();
	}

	return basis;
}

void TorsionBasis::absorbVector(const float *vec, int n)
{
	for (size_t i = 0; i < torsionCount(); i++)
	{
		float torsion = torsionForVector(i, vec, n);
		_torsions[i]->setRefinedAngle(torsion);
		_angles[i].angle = torsion;
	}

}

int TorsionBasis::addTorsion(BondTorsion *torsion)
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

	return _torsions.size() - 1;
}

void TorsionBasis::prepare()
{

}
