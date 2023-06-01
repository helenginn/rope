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

#include "CompareDistances.h"
#include <vagabond/utils/svd/PCA.h>
#include "Atom.h"

CompareDistances::CompareDistances(AtomPosMap &aps) : _aps(aps)
{

}

bool CompareDistances::acceptable(Atom *atom)
{
	return atom->isReporterAtom();
}

PCA::Matrix CompareDistances::matrix()
{
	PCA::Matrix mat{};
	
	int count = 0;
	for (auto it = _aps.begin(); it != _aps.end(); it++)
	{
		if (acceptable(it->first))
		{
			_atoms.push_back(it->first);
			count++;
		}
	}
	
	setupMatrix(&mat, count, count);
	int i = 0; int j = 0;
	
	for (auto it = _aps.begin(); it != _aps.end(); it++)
	{
		if (!acceptable(it->first))
		{
			continue;
		}

		glm::vec3 x = it->second.ave;
		glm::vec3 p = it->second.target;

		for (auto jt = _aps.begin(); jt != _aps.end(); jt++)
		{
			if (it == jt || jt->first->atomName() != "CA")
			{
				continue;
			}

			glm::vec3 y = jt->second.ave;
			glm::vec3 q = jt->second.target;

			float expected = glm::length(p - q);
			float acquired = glm::length(x - y);
			
			float diff = expected - acquired;
			mat[i][j] = diff;
			mat[j][i] = diff;
			j++;
		}
		i++;
		j = 0;
	}
	
	return mat;
}
