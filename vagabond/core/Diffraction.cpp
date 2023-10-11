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

#include "Diffraction.h"
#include "ArbitraryMap.h"
#include <gemmi/symmetry.hpp>

Diffraction::Diffraction(int nx, int ny, int nz) 
: Grid<VoxelDiffraction>(nx, ny, nz),
TransformedGrid<VoxelDiffraction>(nx, ny, nz)
{

}

Diffraction::Diffraction(ArbitraryMap *map)
: Grid<VoxelDiffraction>(),
TransformedGrid<VoxelDiffraction>(0, 0, 0)
{
	setDimensions(map->nx(), map->ny(), map->nz(), false);
	bool flip = (map->status() == ArbitraryMap::Real);
	
	if (flip)
	{
		map->fft();
	}

	for (size_t i = 0; i < nn(); i++)
	{
		element(i).value[0] = map->element(i)[0];
		element(i).value[1] = map->element(i)[1];
	}
	
	if (flip)
	{
		map->fft();
	}
	
	setRealMatrix(map->frac2Real());
}

Diffraction::Diffraction(RefList &list) 
: Grid<VoxelDiffraction>(0, 0, 0),
TransformedGrid<VoxelDiffraction>(0, 0, 0)
{
	_list = &list;

	Reflection::HKL hkl = list.maxSymHKL();
	
	for (size_t i = 0; i < 3; i++)
	{
		hkl[i] = 2 * hkl[i] + 1;
	}

	this->setDimensions(hkl[0], hkl[1], hkl[2]);
	setRealMatrix(_list->frac2Real());
	
	populateReflections();
}

void Diffraction::populateReflections()
{
	for (size_t i = 0; i < nn(); i++)
	{
		element(i).setAmplitudePhase(NAN, NAN);
	}

	for (size_t i = 0; i < _list->reflectionCount(); i++)
	{
		_list->addReflectionToGrid(this, i);
	}

	populateSymmetry();
}

void Diffraction::populateSymmetry()
{
	const gemmi::SpaceGroup *spg;
	spg = gemmi::find_spacegroup_by_name(_list->spaceGroupName());
	gemmi::GroupOps grp = spg->operations();

	gemmi::ReciprocalAsu asu(spg);

	auto lmb = [this, grp, asu](int h, int k, int l)
	{
		long idx = index(h, k, l);
		gemmi::Op::Miller miller{h, k, l};

		std::pair<gemmi::Op::Miller, int> result = asu.to_asu(miller, grp);
		int symop = (result.second - 1) / 2;

		gemmi::Op op = grp.get_op(symop);
		glm::vec3 tr{};

		for (size_t i = 0; i < 3; i++)
		{
			tr[i] = (float)op.tran[i] / 24.;
		}

		long asu_idx = index(result.first[0], result.first[1], result.first[2]);

		float amp = element(asu_idx).amplitude();
		float myPhase = element(asu_idx).phase();
		myPhase *= (result.second % 2 == 0 ? -1 : 1);

		/* rotation */
		float shift = (float)h * tr[0];
		shift += (float)k * tr[1];
		shift += (float)l * tr[2];
		shift = fmod(shift, 1.);

		float newPhase = myPhase + shift * 360.;
		element(idx).setAmplitudePhase(amp, newPhase);
	};

	do_op_on_centred_index(lmb);
}

size_t Diffraction::reflectionCount()
{
	return _list->reflectionCount() * _list->symOpCount() * 2;
}

std::string Diffraction::spaceGroupName()
{
	return _list->spaceGroupName();
}
