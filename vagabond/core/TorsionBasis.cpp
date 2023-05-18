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
#include "Parameter.h"
#include "SimpleBasis.h"
#include "ConcertedBasis.h"
#include "OnPathBasis.h"
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
	else if (type == TorsionBasis::TypeOnPath)
	{
		OnPathBasis *cb = new OnPathBasis();
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
	for (size_t i = 0; i < parameterCount(); i++)
	{
		if (mask && !mask[i])
		{
			continue;
		}

		float torsion = parameterForVector(i, vec, n);
		
//		std::cout << _params[i]->desc() << " " << torsion << std::endl;
		
		_params[i]->setValue(torsion);
		_angles[i].angle = torsion;
		
		_params[i]->setRefined(true);
	}

}

int TorsionBasis::addParameter(Parameter *param, Atom *atom)
{
	if (param == nullptr || param->isConstrained())
	{
		return -1;
	}
	
	for (size_t i = 0; i < _params.size(); i++)
	{
		if (_params[i] == param)
		{
			return i;
		}
	}

	_params.push_back(param);
	_atoms.push_back(atom);

	return _params.size() - 1;
}

void TorsionBasis::prepare(int dims)
{

}

int TorsionBasis::indexForParameter(Parameter *p)
{
	int i = 0;
	for (Parameter *param : _params)
	{
		if (p == param)
		{
			return i;
		}
		i++;
	}
	return -1;
}

void TorsionBasis::trimParametersToUsed(std::set<Parameter *> &params)
{
	bool again = true;
	
	while (again)
	{
		again = false;
		for (Parameter *p : params)
		{
			int idx = indexForParameter(p);

			if (idx < 0)
			{
				params.erase(p); // iterator could be invalidated
				again = true; 
				break;
			}
		}
	}
}

std::vector<int> TorsionBasis::grabIndices(const std::set<Parameter *> &params)
{
	std::vector<int> indices;
	for (Parameter *p : params)
	{
		int idx = indexForParameter(p);
		
		if (idx >= 0)
		{
			indices.push_back(idx);
		}
	}
	
	return indices;
}
