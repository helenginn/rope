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

Coord::Interpolate<float> 
SimpleBasis::valueForParameter(BondSequence *seq, int tidx)
{
	if (tidx < 0)
	{
		return [](const Coord::Get &coord) { return 0; };
	}
	
	TorsionAngle &ta = _angles[tidx];
	float &angle = ta.angle;
	if (!ta.mask)
	{
		return [angle](const Coord::Get &coord) { return angle; };
	}
	else
	{
		return [angle, tidx](const Coord::Get &coord)
		{
			return angle + coord(tidx);
		};
	}
}

/*
float SimpleBasis::parameterForVector(BondSequence *seq, int idx)
{
	if (idx < 0)
	{
		return 0;
	}
	
	TorsionAngle &ta = _angles[idx];
	if (n == 0 || !ta.mask)
	{
		return ta.angle;
	}
	
	if (idx < n)
	{
		return ta.angle + coord(idx);
	}
	
	return ta.angle;
}
*/

void SimpleBasis::prepare(int dims)
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

