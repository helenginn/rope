// vagabond
// Copyright (C) 2022 Helen Ginn
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

#include <vagabond/utils/glm_import.h>
#include "BulkSolvent.h"
#include "grids/Diffraction.h"
#include "grids/ArbitraryMap.h"
#include "grids/BulkMask.h"
#include <gemmi/symmetry.hpp>

BulkSolvent::BulkSolvent(Diffraction *diffraction, const AtomPosMap &all_atoms)
: _atoms(all_atoms)
{
	_model = diffraction;
}

auto next_position(BulkMask *mask, const std::string &spg_name)
{
	const gemmi::SpaceGroup *spg = nullptr;
	spg = gemmi::find_spacegroup_by_name(spg_name);

	gemmi::GroupOps grp = spg->operations();
	std::vector<gemmi::Op> expanded = grp.all_ops_sorted();

	glm::mat3x3 toReal = mask->frac2Real();
	glm::mat3x3 toFrac = glm::inverse(toReal);

	return [expanded, toFrac, toReal]
	(int i, bool &ok, const glm::vec3 &start) -> glm::vec3
	{
		ok = (i < expanded.size());
		glm::vec3 frac = toFrac * start;
		if (!ok) return {};
		std::array<double, 3> xyz = {frac.x, frac.y, frac.z};
		const gemmi::Op &op = expanded[i];
		std::array<double, 3> result = op.apply_to_xyz(xyz);
		glm::vec3 moved = {result[0], result[1], result[2]};
		moved = toReal * moved;
		return moved;
	};
};

template <class Func>
bool advance_all_positions(const Func &next, WithPos &pos)
{
	for (glm::vec3 &sample : pos.samples)
	{
		bool ok = false;
		glm::vec3 advanced = next(ok, sample);

		if (!ok) return false;
		sample = advanced;
	}

	return true;
};

BulkMask *fillBulk(Diffraction *model, const AtomPosMap &atoms, float spacing)
{
	BulkMask *bulk = new BulkMask(model, spacing);
	auto next = next_position(bulk, model->spaceGroupName());
	
	int n = 0;
	while (true)
	{
		auto next_n = [next, n](bool &ok, const glm::vec3 &sample)
		{
			return next(n, ok, sample);
		};

		bool ok = false;
		for (auto it = atoms.begin(); it != atoms.end(); it++)
		{
			WithPos copy = it->second;
			ok = advance_all_positions(next_n, copy);
			
			if (!ok)
			{
				break;
			}

			bulk->addPosList(it->first, copy);
		}

		if (!ok)
		{
			break;
		}

		n++;
	}

	return bulk;
}

Diffraction *BulkSolvent::operator()()
{
	ArbitraryMap *solvent = new ArbitraryMap(*_model);
	solvent->setupFromDiffraction(); // wasteful
	solvent->multiply(0);

	BulkMask *bulk = fillBulk(_model, _atoms, _spacing);
	bulk->contract();
	
	ArbitraryMap *bulkMap = new ArbitraryMap();
	bulkMap->setDimensions(bulk->nx(), bulk->ny(), bulk->nz(), false);
	bulkMap->makePlans();
	bulkMap->setRealMatrix(bulk->frac2Real());
	bulkMap->setStatus(ArbitraryMap::Real);
	bulkMap->clear();

	bulkMap->do_op_on_each_1d_index
	([bulkMap, bulk](long int &idx)
	 {
		bulkMap->element(idx)[0] = bulk->elementValue(idx);

	 });

	delete bulk;
	
	*solvent += *bulkMap;

	Diffraction *recip_solvent = new Diffraction(solvent);
	recip_solvent->applySymmetry(_model->spaceGroupName());

	return recip_solvent;
}
