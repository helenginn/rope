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
ConcertedBasis::valueForParameter(BondSequence *seq, int tidx,
                                  const Coord::Get &coord, int n)
{
	if (tidx < 0)
	{
		return [](const Coord::Get &) { return 0;};
	}

	TorsionAngle &ta = _angles[tidx];

	if (n == 0 || !ta.mask)
	{

		Coord::Interpolate<float> ret = [ta](const Coord::Get &)
		{
			return ta.angle;
		};

		return ret;
	}
	
	Coord::Interpolate<float> sum = fullContribution(seq, tidx, coord, n);

	return sum;
}

// tidx: torsion angle
// vec/n: vector you have to generate torsion angle
float ConcertedBasis::parameterForVector(BondSequence *seq,
                                         int tidx, const Coord::Get &coord, 
                                         int n)
{
	Coord::Interpolate<float> get_angle = valueForParameter(seq, tidx, coord, n);
	if (!get_angle)
	{
		return 0;
	}

	return get_angle(coord);
}

void ConcertedBasis::supplyMask(std::vector<bool> mask)
{
	_refineMask = mask;
}

void ConcertedBasis::setupAngleList()
{
	_angles.clear();

	for (size_t i = 0; i < _params.size(); i++)
	{
		float start = _params[i]->value();
		bool mask = !_params[i]->isConstrained();
		
		if (i < _refineMask.size())
		{
			mask &= _refineMask[i];
		}

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

void ConcertedBasis::prepareSVD()
{
	freeSVD(&_svd);
	setupSVD(&_svd, parameterCount());

	for (size_t i = 0; i < parameterCount(); i++)
	{
		for (size_t j = 0; j <= i; j++)
		{
			float diff = (i == j);

			_svd.u.ptrs[i][j] = diff;
			_svd.u.ptrs[j][i] = diff;
		}
	}
	
	try
	{
		runSVD(&_svd);
	}
	catch (std::runtime_error &err)
	{
		return;
	}

	reorderSVD(&_svd);
}

bool ConcertedBasis::reverseLookup(Instance *inst, int axis,
                                   const RTAngles &angles)
{
	bool changed = false;

	for (size_t j = 0; j < angles.size(); j++)
	{
		for (size_t i = 0; i < _params.size(); i++)
		{
			Parameter *t = _params[i];

			const ResidueTorsion &rt = angles.c_rt(j);
			const Residue *local = inst->localResidueForResidueTorsion(rt);
			if (local->id() != t->residueId())
			{
				continue;
			}
			
			const std::string &desc = rt.torsion().desc();

			if (!t->hasDesc(desc))
			{
				continue;
			}

			float value = angles.c_storage(j);

			if (value != value)
			{
				value = 0;
			}

			_svd.u[i][axis] = value;
			changed = true;
		}
	}

	return changed;
}

bool ConcertedBasis::fillFromInstanceList(Instance *instance, int axis,
                                          const RTAngles &angles)
{
	prepare();
	
	if (angles.size() == 1)
	{
		bool result = reverseLookup(instance, axis, angles);
		return result;
	}

	std::cout << "Adding " << instance->id() << " axis " << axis << ", "
	<< angles.size() << " angles" << std::endl;
	
	for (size_t i = 0; i < _params.size(); i++)
	{
		Parameter *t = _params[i];
		
		float value = instance->valueForTorsionFromList(t, angles);
		
		if (value != value)
		{
			value = 0;
			_missing.push_back(t);
		}
		
		_svd.u[i][axis] = value;
	}
	
	return true;
}

void ConcertedBasis::prepare(int dims)
{
	_dims = dims;
	setupAngleList();
	
	if (!_custom)
	{
		prepareSVD();
	}
}

size_t ConcertedBasis::activeBonds()
{
	return parameterCount();
}

Coord::Interpolate<float> 
ConcertedBasis::fullContribution(BondSequence *seq, int tidx, 
                                 const Coord::Get &coord, int n)
{
	// tidx = parameter in _params vector
	// contr_idx = parameter in the svd matrix, excluding constrained

	if (tidx < 0 || tidx > _params.size())
	{
		return [](const Coord::Get &) { return 0;};
	}

	Coord::Interpolate<float> all;
	float angle = _angles[tidx].angle;

	all = [seq, angle, tidx, n, this](const Coord::Get &coord)
	{
		float ret = 0;
		for (size_t axis = 0; axis < n; axis++)
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

