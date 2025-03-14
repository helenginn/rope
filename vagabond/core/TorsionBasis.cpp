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

TorsionBasis::TorsionBasis()
{

}

void TorsionBasis::absorbVector(const Coord::Get &coord)
{
	for (size_t i = 0; i < parameterCount(); i++)
	{
		float torsion = coord(i);
		
		_angles[i].angle += torsion;
		_params[i]->setValue(_angles[i].angle);
		_params[i]->setRefined(true);
	}

}

int TorsionBasis::addParameter(Parameter *param, Atom *atom)
{
	if (param == nullptr)
	{
		return -1;
	}
	
	if (_param2Idx.count(param))
	{
		return _param2Idx.at(param);
	}

	_param2Idx[param] = _params.size();
	_params.push_back(param);
	_atoms.push_back(atom);

	return _params.size() - 1;
}

void TorsionBasis::prepare()
{
	_angles.clear();

	for (size_t i = 0; i < _params.size(); i++)
	{
		float start = _params[i]->value();
		bool mask = !_params[i]->isConstrained();
		TorsionAngle ta = {start, mask};
		_angles.push_back(ta);
	}

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

