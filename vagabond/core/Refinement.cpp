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

#include "File.h"
#include "Warp.h"
#include "Model.h"
#include "Entity.h"
#include "AtomMap.h"
#include "Instance.h"
#include "MolRefiner.h"
#include "Refinement.h"
#include "Diffraction.h"
#include "ArbitraryMap.h"
#include "MetadataGroup.h"
#include "SymmetryExpansion.h"

#include "Diffraction.h"
#include "MtzFile.h"

#include <fstream>

Refinement::Refinement()
{

}

void Refinement::setup()
{
	_model->load();

	prepareInstanceDetails();
	setupRefiners();
}

void Refinement::loadDiffraction(const std::string &filename)
{
	File *file = File::loadUnknown(filename);

	if (!file)
	{
		return;
	}

	File::Type type = file->cursoryLook();

	if (type & File::Reflections)
	{
		Diffraction *diff = file->diffractionData();
		_diff = diff;

		_map = new ArbitraryMap(*diff);
		_map->setupFromDiffraction();
	}

	delete file;
}

void Refinement::prepareInstanceDetails()
{
	std::vector<Instance *> instances = _model->instances();
	for (Instance *inst : instances)
	{
		prepareInstance(inst);
	}
}

void Refinement::setupRefiners()
{
	for (Refine::Info &info : _molDetails)
	{
		setupRefiner(info);
	}
}

void Refinement::prepareInstance(Instance *mol)
{
	Refine::Info info;
	info.instance = mol;
	info.mol_id = mol->id();

	_molDetails.push_back(info);
}

void Refinement::setupRefiner(Refine::Info &info)
{
	Instance *mol = info.instance;
	if (!mol->hasSequence())
	{
		return;
	}
	
	const int dims = 3;
	
	info.samples = 120;
	info.master_dims = 3;
	info.warp = Warp::warpFromFile(info.instance, "test.json");
	MolRefiner *mr = new MolRefiner(_map, &info);
	_molRefiners[mol] = mr;
}

void Refinement::play()
{
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.instance];

		if (mr)
		{
			mr->runEngine();
		}
	}
	
	calculatedMapAtoms();
}

ArbitraryMap *Refinement::calculatedMapAtoms(Diffraction **reciprocal,
                                             float max_res)
{
	ArbitraryMap *arb = new ArbitraryMap(*_map);
	arb->clear();
	
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.instance];
		if (!mr)
		{
			continue;
		}

		Result *result = mr->submitJobAndRetrieve({});
		AtomMap &map = *result->map;
		ArbitraryMap *partial = map();
		*arb += *partial;
		delete partial;
		result->destroy();
	}

	const gemmi::SpaceGroup *spg = nullptr;
	spg = gemmi::find_spacegroup_by_name(_map->spaceGroupName());
	gemmi::GroupOps grp = spg->operations();
	
	// delete me later
	Diffraction *diff = new Diffraction(arb);

	MtzFile file("");
	file.setMap(diff);
	SymmetryExpansion::apply(diff, spg, max_res);
	file.write_to_file("symm.mtz", max_res);
	
	if (reciprocal)
	{
		*reciprocal = diff;
	}
	
	return arb;
}

