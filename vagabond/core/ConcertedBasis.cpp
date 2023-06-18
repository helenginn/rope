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

// tidx: torsion angle
// vec/n: vector you have to generate torsion angle
float ConcertedBasis::parameterForVector(BondSequence *seq,
                                         int tidx, const AcquireCoord &coordinate, 
                                         int n)
{
	if (tidx < 0)
	{
		return 0;
	}

	if (n >= tidx && _angles.size() == 0)
	{
		prepare();
	}

	TorsionAngle &ta = _angles[tidx];

	if (n == 0 || !ta.mask)
	{
		return ta.angle;
	}
	
	int contracted_tidx = _idxs[tidx];
	float sum = fullContribution(seq, tidx, coordinate, n);
	
	Parameter *bt = _filtered[contracted_tidx];
	
	sum += ta.angle;

	return sum;
}

void ConcertedBasis::supplyMask(std::vector<bool> mask)
{
	_refineMask = mask;
}

void ConcertedBasis::setupAngleList()
{
	_angles.clear();
	_idxs.clear();
	_nActive = 0;

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
		
		if (mask)
		{
			_filtered.push_back(_params[i]);
			_idxs.push_back(_nActive);
			_nActive++;
		}
		else
		{
			_idxs.push_back(-1);
		}
	}
	
	if (_dims == 0)
	{
		_dims = _nActive;
	}

	freeSVD(&_svd);
	setupSVD(&_svd, _nActive, _dims);

}

void ConcertedBasis::prepareSVD()
{
	freeSVD(&_svd);
	setupSVD(&_svd, _nActive);

	for (size_t i = 0; i < _nActive; i++)
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
                                   const std::vector<ResidueTorsion> &list,
                                   const std::vector<Angular> &values)
{
	bool changed = false;

	for (size_t j = 0; j < list.size(); j++)
	{
		for (size_t i = 0; i < _params.size(); i++)
		{
			Parameter *t = _params[i];

			if (_idxs[i] < 0)
			{
				continue;
			}

//			float value = instance->valueForTorsionFromList(t, list, values, found);
			const Residue *local = inst->localResidueForResidueTorsion(list[j]);
			if (local->id() != t->residueId())
			{
				continue;
			}
			
			const std::string &desc = list[j].torsion().desc();

			if (!t->hasDesc(desc))
			{
				continue;
			}

			float value = values[j];

			if (value != value)
			{
				value = 0;
			}

			int idx = _idxs[i];
			_svd.u[idx][axis] = value;
			changed = true;
		}
	}

	return changed;
}

bool ConcertedBasis::fillFromInstanceList(Instance *instance, int axis,
                                          const std::vector<ResidueTorsion> &list,
                                          const std::vector<Angular> &values)
{
	prepare();
	std::vector<bool> found(list.size(), false);
	
	if (values.size() == 1)
	{
		bool result = reverseLookup(instance, axis, list, values);
		return result;
	}

	std::cout << "Adding " << instance->id() << " axis " << axis << ", "
	<< values.size() << " values" << std::endl;
	
	for (size_t i = 0; i < _params.size(); i++)
	{
		Parameter *t = _params[i];
		
		if (_idxs[i] < 0)
		{
			continue;
		}

		float value = instance->valueForTorsionFromList(t, list, values, found);
		
		if (value != value)
		{
			value = 0;
			_missing.push_back(t);
		}
		
		int idx = _idxs[i];
		_svd.u[idx][axis] = value;
	}
	
	for (size_t i = 0; i < found.size(); i++)
	{
		if (!found[i])
		{
			_unusedId = list[i].master();
		}
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
	return _nActive;
}

float ConcertedBasis::fullContribution(BondSequence *seq, int tidx, 
                                       const AcquireCoord &coordinate, int n)
{
	if (tidx < 0 || tidx > _svd.u.rows)
	{
		return 0;
	}

	float ret = 0;

	// each n is an axis
	for (size_t i = 0; i < n; i++)
	{
		float add = contributionForAxis(seq, tidx, i, coordinate);
		ret += add;
	}

	return ret;
}

float ConcertedBasis::contributionForAxis(BondSequence *seq,
                                          int tidx, int i, const AcquireCoord &coordinate)
{
	if (i > _svd.u.rows)
	{
		return 0;
	}

	double svd = (_svd.u[tidx][i]);
	const float &custom = coordinate(i);
	float add = svd * custom;
	return add;
}

