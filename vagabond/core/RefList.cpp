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

#include "RefList.h"
#include "matrix_functions.h"
#include <iostream>

RefList::RefList(std::vector<Reflection> &refls)
{
	_refls = refls;
}

HKL RefList::maxHKL()
{
	HKL hkl{};
	
	for (size_t i = 0; i < _refls.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (abs(_refls[i].hkl[j]) > hkl[j])
			{
				hkl[j] = abs(_refls[i].hkl[j]);
			}
		}

	}

	return hkl;
}

const double RefList::resolutionOf(const int i) const
{
	glm::vec3 v = glm::vec3(_refls[i].hkl.h, _refls[i].hkl.k,
	                        _refls[i].hkl.l);

	v = _recip2Frac * v;
	double l = glm::length(v);
	return 1 / l;
}

void RefList::setUnitCell(std::array<double, 6> &cell)
{
	_cell = cell;
	_frac2Real = mat3x3_from_unit_cell(cell[0], cell[1], cell[2],
	                                   cell[3], cell[4], cell[5]);
	_recip2Frac = glm::inverse(_frac2Real);
}
