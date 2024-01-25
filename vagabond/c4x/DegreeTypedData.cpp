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


#ifndef __vagabond__DegreeTypedData__cpp__
#define __vagabond__DegreeTypedData__cpp__

#include "DegreeTypedData.h"
#include <iostream>

using std::isfinite;

template <class Header>
void DegreeTypedData<Header>::matchDegrees(Array &next)
{
	for (size_t i = 0; i < _length; i++)
	{
		if (next[i] != next[i] || !valid(next[i]))
		{
			next[i] = NAN;
			continue;
		}
		
		size_t j = 0;
		typename TypedData<Angular, Header>::Array *master = nullptr;
		
		if (this->vectorCount())
		{
			master = &_entries[j].vector;

			for (j = 0; j < TypedData<Angular, Header>::vectorCount(); j++)
			{
				master = &_entries[j].vector;

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
void DegreeTypedData<Header>::addArray(std::string name, Array next)
{
	if (this->vectorCount() > 0)
	{
		matchDegrees(next);
	}
	
	TypedData<Angular, Header>::addArray(name, next);

	if (_entries.size() <= 1)
	{
		return;
	}
}

#endif
