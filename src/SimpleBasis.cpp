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

#include "SimpleBasis.h"
#include "BondTorsion.h"

SimpleBasis::SimpleBasis() : TorsionBasis()
{

}

float SimpleBasis::torsionForVector(int idx, const float *vec, int n)
{
	if (idx < 0)
	{
		return 0;
	}

	TorsionAngle &ta = _angles[idx];
	if (n == 0)
	{
		return ta.angle;
	}

//	double start = _torsions[idx]->startingAngle();
	
	if (n == 0 || !ta.mask)
	{
		return ta.angle;
	}
	
	if (idx < n)
	{
		return ta.angle + vec[idx];
	}
	
	return ta.angle;
}

void SimpleBasis::absorbVector(const float *vec, int n)
{
	for (size_t i = 0; i < n; i++)
	{
		float torsion = torsionForVector(i, vec, n);
		_torsions[i]->setRefinedAngle(torsion);
		_angles[i].angle = torsion;
	}

}

void SimpleBasis::prepare()
{
	_angles.clear();
	for (size_t i = 0; i < _torsions.size(); i++)
	{
		float start = _torsions[i]->startingAngle();
		bool mask = !_torsions[i]->isConstrained();
		TorsionAngle ta = {start, mask};
		_angles.push_back(ta);
	}

}
