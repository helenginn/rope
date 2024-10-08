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
#include "Unit.h"
#include "Refinement.h"
#include "Wiggler.h"

#include <fstream>

Refinement::Refinement()
{

}

void Refinement::setup()
{
	_model->load();
	_model->currentAtoms()->recalculate();

	prepareInstanceDetails();
}

void Refinement::loadDiffraction(const std::string &filename)
{
	Diffraction *diff = _model->getDiffraction();
	_data = diff;

	_map = new ArbitraryMap(*diff);
	_map->setupFromDiffraction();
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
		Refine::Module mods = Refine::Module(Refine::Warp | 
		                                       Refine::ImplicitB |
		                                     Refine::Barycentric |
		                                       Refine::Translate |
		                                       Refine::Rotate);

		if (!inst->hasSequence())
		{
			mods = Refine::Module(Refine::None);
		}

		inst->load();

		Refine::Info &info = prepareInstance(inst);
		setupRefiner(info);

		Wiggler wiggler = Wiggler(info, info.refiner->sampler());
		wiggler.setModules(mods);
		wiggler();
	}
	
	if (single_atoms.size())
	{
		{
			Refine::Info info;
			info.max_res = _data->maxResolution();
			for (Instance *inst : single_atoms)
			{
				inst->load();
				info.instances.push_back(inst);
			}

			_molDetails.push_back(info);
		}
		
		Refine::Info &network = _molDetails.back();
		network.max_res = _data->maxResolution();
		network.samples = 120;
		network.master_dims = 3;

		Unit *unit = new Unit(_map, &network);
		network.refiner = unit;

		Wiggler wiggler = Wiggler(network, network.refiner->sampler());
		wiggler.setModules(Refine::Translate);
		wiggler();

	}
}

Refine::Info &Refinement::prepareInstance(Instance *mol)
{
	Refine::Info info;
	info.instances.push_back(mol);
	_molDetails.push_back(info);
	return _molDetails.back();
}

void Refinement::setupRefiner(Refine::Info &info)
{
	if (info.instances.size() == 0) return;
	Instance *mol = info.instances[0];
	std::string file = (mol->hasSequence() ? mol->id() + ".json" : "");
	
	info.samples = 120;
	info.master_dims = 3;
	info.max_res = _data->maxResolution();
	std::cout << info.max_res << " Ang" << std::endl;
	info.warp = Warp::warpFromFile(mol, file);
	Unit *unit = new Unit(_map, &info);
	info.refiner = unit;
}

void Refinement::play()
{
	for (Refine::Info &info  : _molDetails)
	{
		Unit *unit = info.refiner;

		if (unit)
		{
			unit->runEngine();
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
		Unit *unit = info.refiner;
		if (!unit)
		{
			continue;
		}

		unit->prepareResources();
		Result *result = unit->submitJobAndRetrieve({});
		AtomMap &map = *result->map;
		ArbitraryMap *partial = map();
		arb->addFromOther(*partial);
		delete partial;
		result->destroy();
	}

	if (reciprocal)
	{
		Diffraction *diff = new Diffraction(arb);
		diff->applySymmetry(_map->spaceGroupName());
		*reciprocal = diff;
	}

	return arb;
}

void Refinement::swapMap(ArbitraryMap *map)
{
	for (Refine::Info &info  : _molDetails)
	{
		Unit *unit = info.refiner;
		if (!unit)
		{
			continue;
		}

		unit->changeMap(map);
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

