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
#include "grids/Diffraction.h"
#include "matrix_functions.h"
#include <gemmi/symmetry.hpp>
#include <iostream>

RefList::RefList(const std::vector<Reflection> &refls)
{
	_refls = refls;
	_spgName = "P 1";
}

RefList::~RefList()
{

}

Reflection::HKL RefList::maxSymHKL()
{
	const gemmi::SpaceGroup *spg;
	spg = gemmi::find_spacegroup_by_name(spaceGroupName());
	gemmi::GroupOps grp = spg->operations();

    Reflection::HKL true_max = Reflection::HKL{};
	
	for (size_t j = 0; j < reflectionCount(); j++)
	{
		Reflection::HKL &hkl = _refls[j].hkl;
		gemmi::Op::Miller m = {hkl[0], hkl[1], hkl[2]};
		for (int l = 0; l < grp.sym_ops.size(); l++)
		{
			gemmi::Op op = grp.get_op(l);
			gemmi::Op::Miller n = op.apply_to_hkl(m);

			for (size_t k = 0; k < 3; k++)
			{
				true_max[k] = std::max(true_max[k], abs(n[k]));
			}
		}
	}

	return true_max;
}

const double RefList::resolutionOf(glm::vec3 v) const
{
	double l = glm::length(v);
	return 1 / l;
}

const glm::vec3 RefList::reflAsIndex(const int i) const
{
	glm::vec3 v = glm::vec3(_refls[i].hkl.h, _refls[i].hkl.k,
	                        _refls[i].hkl.l);
	
	return v;
}

const glm::vec3 RefList::reflAsFraction(const int i) const
{
	glm::vec3 v = _recip2Frac * reflAsIndex(i);
	return v;
}

const double RefList::resolutionOf(const int i) const
{
	glm::vec3 v = reflAsFraction(i);
	return resolutionOf(v);
}

void RefList::setUnitCell(std::array<double, 6> &cell)
{
	_cell = cell;
	_frac2Real = mat3x3_from_unit_cell(cell[0], cell[1], cell[2],
	                                   cell[3], cell[4], cell[5]);
	_recip2Frac = glm::inverse(_frac2Real);
}

void RefList::addReflectionToGrid(Diffraction *diff, int refl)
{
	Reflection::HKL &orig = _refls[refl].hkl;
	const int &h = orig.h;
	const int &k = orig.k;
	const int &l = orig.l;
	if (_refls[refl].free)
	{
		return;
	}

	float phase = _refls[refl].phi;
	const float &f = _refls[refl].f;

	VoxelDiffraction &v = diff->element(h, k, l);
	v.setAmplitudePhase(f, phase);
}

