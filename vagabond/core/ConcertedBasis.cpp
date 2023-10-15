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
#include "ConcertedBasis.h"
#include "BondSequence.h"
#include "Parameter.h"
#include "Polymer.h"

ConcertedBasis::ConcertedBasis() : TorsionBasis()
{
	setupSVD(&_svd, 1);
}

ConcertedBasis::~ConcertedBasis()
{
	freeSVD(&_svd);
}

Coord::Interpolate<float>
ConcertedBasis::valueForParameter(BondSequence *seq, int tidx)
{
	if (tidx < 0)
	{
		return [](const Coord::Get &) { return 0;};
	}

	TorsionAngle &ta = _angles[tidx];

	if (!ta.mask)
	{

		Coord::Interpolate<float> ret = [ta](const Coord::Get &)
		{
			return ta.angle;
		};

		return ret;
	}
	
	Coord::Interpolate<float> sum = fullContribution(seq, tidx);

	return sum;
}

void ConcertedBasis::setupAngleList()
{
	_angles.clear();

	for (size_t i = 0; i < _params.size(); i++)
	{
		float start = _params[i]->value();
		bool mask = !_params[i]->isConstrained();
		
		TorsionAngle ta = {start, mask};
		_angles.push_back(ta);
	}
	
	if (_dims == 0)
	{
		_dims = parameterCount();
	}

	freeSVD(&_svd);
	setupSVD(&_svd, parameterCount(), _dims);

}

bool ConcertedBasis::fillFromInstanceList(Instance *instance, int axis,
                                          RTAngles angles)
{
	prepare();
	angles.attachInstance(instance);
	angles.filter_according_to(parameters());
	
	for (int i = 0; i < parameterCount(); i++)
	{
		_svd.u[i][axis] = angles.storage(i);
	}
	
	return true;
}

void ConcertedBasis::prepare(int dims)
{
	_dims = dims;
	setupAngleList();
}

Coord::Interpolate<float> 
ConcertedBasis::fullContribution(BondSequence *seq, int tidx)
{
	if (tidx < 0 || tidx > _params.size())
	{
		return [](const Coord::Get &) { return 0;};
	}

	Coord::Interpolate<float> all;
	float angle = _angles[tidx].angle;

	all = [seq, angle, tidx, this](const Coord::Get &coord)
	{
		float ret = 0;
		for (size_t axis = 0; axis < seq->nCoords(); axis++)
		{
			float add = this->contributionForAxis(seq, tidx, axis, coord);
			ret += add;
		}

		return ret + angle;
	};
	
	return all;

}

float ConcertedBasis::contributionForAxis(BondSequence *seq, 
                                          int tidx, int axis, 
                                          const Coord::Get &coord) const
{
	if (axis > _svd.u.rows)
	{
		return 0;
	}

	double svd = (_svd.u[tidx][axis]);
	const float &custom = coord(axis);
	float add = svd * custom;
	
	return add;
}

