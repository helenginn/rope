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

#include "ConcertedBasis.h"
#include "BondTorsion.h"

ConcertedBasis::ConcertedBasis() : TorsionBasis()
{

}

void ConcertedBasis::supplyMask(std::vector<bool> mask)
{
	_refineMask = mask;

}

void ConcertedBasis::setupAngleList()
{
	_angles.clear();
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
			_nActive++;
		}
	}
}

void ConcertedBasis::prepare()
{
	setupAngleList();

}

size_t ConcertedBasis::activeBonds()
{
	return _nActive;
}
