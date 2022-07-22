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

template <class Header>
void DegreeDataGroup<Header>::matchDegrees(Array &next)
{
	for (size_t i = 0; i < _length; i++)
	{
		if (next[i] != next[i] || !isfinite(next[i]))
		{
			next[i] = NAN;
			continue;
		}
		
		size_t j = 0;
		Array *master = &_vectors[j];
		for (j = 0; j < DataGroup<float, Header>::vectorCount(); j++)
		{
			master = &_vectors[j];

			if ((*master)[i] == (*master)[i] && isfinite((*master)[i]))
			{
				break;
			}
		}

		while (next[i] < (*master)[i] - 180)
		{
			next[i] += 360;
		}

		while (next[i] >= (*master)[i] + 180)
		{
			next[i] -= 360;
		}
	}
}

template <class Header>
float DegreeDataGroup<Header>::difference(int m, int n, int j)
{
	float deg = (_diffs[n][j] - _diffs[m][j]) * _stdevs[j];

	while (deg < -180)
	{
		deg += 360;
	}

	while (deg >= 180)
	{
		deg -= 360;
	}
	
	return deg;
}

template <class Header>
void DegreeDataGroup<Header>::addArray(std::string name, Array next)
{
	if (_vectors.size() > 0)
	{
		matchDegrees(next);
	}
	
	DataGroup<float, Header>::addArray(name, next);

	if (_vectors.size() <= 1)
	{
		return;
	}
}

#endif
