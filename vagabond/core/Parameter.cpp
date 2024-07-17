// vagabond
// Copyright (C) 2022 Helen Ginn
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

#include "Parameter.h"
#include "Atom.h"

std::set<Parameter *> Parameter::relatedParameters(bool close) const
{
	std::set<Parameter *> params;

	int start = close ? 1 : 0;
	int end = close ? 2 : atomCount();
	for (size_t i = start; i < atomCount() && i <= end; i++)
	{
		Atom *a = atom(i);
		
		for (size_t j = 0; j < a->parameterCount(); j++)
		{
			Parameter *p = a->parameter(j);
			if (!close || p->atomIsCentral(a))
			{
				params.insert(p);
			}
		}
	}
	
	return params;
}

std::set<Parameter *> Parameter::sisters() const
{
	if (!isTorsion())
	{
		return {};
	}

	std::set<Parameter *> params;
	Atom *left = atom(1);
	Atom *right = atom(2);

	for (size_t i = 1; i < atomCount() && i <= 2; i++)
	{
		Atom *a = atom(i);
		
		for (size_t j = 0; j < a->parameterCount(); j++)
		{
			Parameter *p = a->parameter(j);
			
			if (p->isTorsion() && 
			    p->atomIsCentral(left) && p->atomIsCentral(right))
			{
				params.insert(p);
			}
		}
	}
	
	return params;
}
