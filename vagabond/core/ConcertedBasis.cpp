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
#include "BondTorsion.h"

ConcertedBasis::ConcertedBasis() : TorsionBasis()
{

}

ConcertedBasis::~ConcertedBasis()
{
	freeSVD(&_svd);
}

float ConcertedBasis::torsionForVector(int idx, const float *vec, int n)
{
	if (idx < 0)
	{
		return 0;
	}

	if (n >= idx && _angles.size() == 0)
	{
		prepare();
	}

	TorsionAngle &ta = _angles[idx];

	if (n == 0 || !ta.mask)
	{
		return ta.angle;
	}

	float sum = 0;
	float total = 0;

	int contracted = _idxs[idx];
	for (size_t i = 0; i < n; i++)
	{
		if (contracted < 0 || contracted > _svd.u.rows || i > _svd.u.rows)
		{
			continue;
		}
		
		double svd = (_svd.u.ptrs[contracted][i]);
		const float &custom = vec[i];
		
		sum += svd * custom;
		total++;
	}

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

	for (size_t i = 0; i < _torsions.size(); i++)
	{
		float start = _torsions[i]->startingAngle();
		bool mask = !_torsions[i]->isConstrained();
		
		if (i < _refineMask.size())
		{
			mask &= _refineMask[i];
		}

		TorsionAngle ta = {start, mask};
		_angles.push_back(ta);
		
		if (mask)
		{
			_filtered.push_back(_torsions[i]);
			_idxs.push_back(_nActive);
			_nActive++;
		}
		else
		{
			_idxs.push_back(-1);
		}
	}
}

void ConcertedBasis::prepareSVD()
{
	freeSVD(&_svd);
	setupSVD(&_svd, _nActive);

	for (size_t i = 0; i < _nActive; i++)
	{
		for (size_t j = 0; j <= i; j++)
		{
			BondTorsion *a = _filtered[i];
			BondTorsion *b = _filtered[j];
			
			float diff = a->similarityScore(b);

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

void ConcertedBasis::prepare()
{
	setupAngleList();
	prepareSVD();
}

size_t ConcertedBasis::activeBonds()
{
	return _nActive;
}

