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


#ifndef __vagabond__DegreeDataGroup__cpp__
#define __vagabond__DegreeDataGroup__cpp__

#include "DegreeDataGroup.h"
#include <iostream>

using std::isfinite;

template <class Header>
void DegreeDataGroup<Header>::matchDegrees(Array &next)
{
	for (size_t i = 0; i < _length; i++)
	{
		if (next[i] != next[i] || !valid(next[i]))
		{
			next[i] = NAN;
			continue;
		}
		
		size_t j = 0;
		Array *master = nullptr;
		
		if (_vectors.size())
		{
			master = &_vectors[j];

			for (j = 0; j < DataGroup<Angular, Header>::vectorCount(); j++)
			{
				master = &_vectors[j];

				if ((*master)[i] == (*master)[i] && valid((*master)[i]))
				{
					break;
				}
			}
		}
		
		float ref = 0;
		
		if (master)
		{
			ref = (*master)[i];
		}

		while (next[i] < ref - 180.f)
		{
			next[i] += 360.f;
		}

		while (next[i] >= ref + 180.f)
		{
			next[i] -= 360.f;
		}
	}
}

template <class Header>
void DegreeDataGroup<Header>::addArray(std::string name, Array next)
{
	if (_vectors.size() > 0)
	{
		matchDegrees(next);
	}
	
	DataGroup<Angular, Header>::addArray(name, next);

	if (_vectors.size() <= 1)
	{
		return;
	}
}

#endif
