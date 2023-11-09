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

#include <list>

#include "Warp.h"
#include "Model.h"
#include "Entity.h"
#include "grids/Diffraction.h"
#include "grids/ArbitraryMap.h"
#include "grids/AtomMap.h"
#include "Babinet.h"
#include "files/MtzFile.h"
#include "Instance.h"
#include "UpdateMap.h"
#include "MolRefiner.h"
#include "Refinement.h"
#include "WarpedRefine.h"
#include "SymmetryExpansion.h"

#include <fstream>

Refinement::Refinement()
{

}

void Refinement::setup()
{
	_model->load();

	prepareInstanceDetails();
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
		_data = diff;

		_map = new ArbitraryMap(*diff);
		_map->setupFromDiffraction();
	}

	delete file;
}

template <typename Filter>
std::list<Instance *> list_from_instances(std::vector<Instance *> &instances,
                                          const Filter &filter)
{
	std::list<Instance *> list;
	
	for (Instance *instance : instances)
	{
		if (filter(instance))
		{
			list.push_back(instance);
		}
	}

	return list;
}

void Refinement::prepareInstanceDetails()
{
	std::vector<Instance *> instances = _model->instances();
	
	std::list<Instance *> single_atoms, multi_atoms;

	single_atoms = list_from_instances(instances, [](Instance *const &inst)
									   {
										   return inst->currentAtoms()->size() == 1;
									   });

	multi_atoms = list_from_instances(instances, [](Instance *const &inst)
									   {
										   return inst->currentAtoms()->size() > 1;
									   });

	std::cout << instances.size() << " instances of which " << single_atoms.size()
	<< " only have one atom." << std::endl;

	for (Instance *inst : multi_atoms)
	{
		if (!inst->hasSequence())
		{
			continue;
		}

		Refine::Info &info = prepareInstance(inst);
		setupRefiner(info);

		WarpedRefine wr = WarpedRefine(inst, info, info.refiner->sampler());
		wr();
	}
}

Refine::Info &Refinement::prepareInstance(Instance *mol)
{
	Refine::Info info;
	info.instance = mol;
	_molDetails.push_back(info);
	return _molDetails.back();
}

void Refinement::setupRefiner(Refine::Info &info)
{
	Instance *mol = info.instance;
	
	info.samples = 120;
	info.master_dims = 3;
	info.warp = Warp::warpFromFile(info.instance, "test.json");
	MolRefiner *mr = new MolRefiner(_map, &info);
	info.refiner = mr;
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
	
	updateMap();
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

		mr->prepareResources();
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
	
	if (reciprocal)
	{
		// delete me later
		Diffraction *diff = new Diffraction(arb);
		SymmetryExpansion::apply(diff, spg, max_res);

		MtzFile file("");
		file.setMap(diff);
		file.write_to_file("symm.mtz", max_res);

		*reciprocal = diff;
	}

	return arb;
}

void Refinement::swapMap(ArbitraryMap *map)
{
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.instance];
		if (!mr)
		{
			continue;
		}

		mr->changeMap(map);
	}

	sendResponse("swap_map", map);
	_map = map;
}

void Refinement::updateMap()
{
	/* currently matched to service crystallography */

	/* calculate the initial model map to required resolution */
	float maxRes = _data->maxResolution();
	Diffraction *recip_model = nullptr;
	ArbitraryMap *real = calculatedMapAtoms(&recip_model, maxRes);
	delete real;

	/* populate with solvent stuff */
	Babinet babinet(recip_model);
	Diffraction *solvated_model = babinet();
	delete recip_model;
	
	/* recombine with data */
	UpdateMap update(_data, solvated_model);
	Diffraction *combined = update();
	
	/* prepare new map for target function */
	ArbitraryMap *map = new ArbitraryMap(*combined);
	map->setupFromDiffraction();
	swapMap(map);

	delete solvated_model;
}

