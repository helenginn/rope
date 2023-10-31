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

#include "Model.h"
#include "Entity.h"
#include "Instance.h"
#include "MolRefiner.h"
#include "Refinement.h"
#include "ArbitraryMap.h"
#include "MetadataGroup.h"
#include "SymmetryExpansion.h"

#include "Diffraction.h"
#include "MtzFile.h"

#include <vagabond/c4x/ClusterSVD.h>
#include <fstream>

Refinement::Refinement()
{

}

void Refinement::setup()
{
	_model->load();

	prepareInstanceDetails();
	loadMap();
	setupRefiners();
}

void Refinement::loadMap()
{
	_map = _model->map();
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

ECluster *Refinement::grabCluster(Entity *entity)
{
	if (_entity2Cluster.count(entity))
	{
		return _entity2Cluster[entity];
	}

	MetadataGroup angles = entity->makeTorsionDataGroup();
	ECluster *cx = new ECluster(angles);
	cx->cluster();
	_entity2Cluster[entity] = cx;

	return cx;
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
	
	const int dims = 2;
	
	int samples = info.samples;

	MolRefiner *mr = new MolRefiner(_map, &info, samples, dims);
	_molRefiners[mol] = mr;
}

void Refinement::play()
{
//	calculatedMapAtoms();
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.instance];
		mr->runEngine();
	}
	
	_model->write("after_refinement.pdb");
}

ArbitraryMap *Refinement::calculatedMapAtoms()
{
	ArbitraryMap *arb = new ArbitraryMap(*_map);
	arb->clear();
	return arb;
	
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.instance];
//		mr->addToMap(arb);
	}

	const gemmi::SpaceGroup *spg = nullptr;
	spg = gemmi::find_spacegroup_by_name(_map->spaceGroupName());
	gemmi::GroupOps grp = spg->operations();
	
	// delete me later
	Diffraction *diff = new Diffraction(arb);

	MtzFile file("");
	file.setMap(diff);
	file.write_to_file("spg1.mtz", 1.5);

	SymmetryExpansion::apply(diff, spg, 1.5);
	file.write_to_file("symm.mtz", 1.5);
	
	return arb;

}

float Refinement::comparisonWithData()
{
	ArbitraryMap *calc = calculatedMapAtoms();
	float result = _model->comparisonWithData(calc);
	std::cout << "Comparison result: " << result << std::endl;
	return result;
}
