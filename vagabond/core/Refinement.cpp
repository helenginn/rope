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
#include "Polymer.h"
#include "MolRefiner.h"
#include "Refinement.h"
#include "MetadataGroup.h"

#include <vagabond/c4x/ClusterSVD.h>

Refinement::Refinement()
{

}

void Refinement::setup()
{
	_model->load();

	preparePolymerDetails();
	loadMap();
	setupRefiners();
}

void Refinement::loadMap()
{
	_map = _model->map();
}

void Refinement::preparePolymerDetails()
{
	for (Polymer &mol : _model->polymers())
	{
		preparePolymer(&mol);
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

void Refinement::preparePolymer(Polymer *mol)
{
	Refine::Info info;
	info.molecule = mol;
	info.mol_id = mol->id();

	ECluster *cluster = grabCluster(mol->entity());

	std::vector<ResidueTorsion> list = cluster->dataGroup()->headers();
	std::vector<Refine::RTA> rtas = Refine::RTA::vector_from_rts(list);
	
	int max = 5;
	
	/* get three top axes from cluster */
	for (size_t i = 0; i < max && i < cluster->rows(); i++)
	{
		std::vector<Angular> vals = cluster->rawVector(i);
		std::vector<Refine::RTA> copy = rtas;
		
		for (size_t j = 0; j < vals.size(); j++)
		{
			copy[j].angle = vals[j];
		}
		
		Refine::Axis axis;
		axis.angles = copy;
		info.axes.push_back(axis);
	}
	
	/* set mean = 0 */
	for (size_t i = 0; i < info.axes.size(); i++)
	{
		info.mean.push_back(0);
	}
	
	_molDetails.push_back(info);
}

void Refinement::setupRefiner(Refine::Info &info)
{
	Polymer *mol = info.molecule;
	ECluster *cluster = grabCluster(mol->entity());

	int dims = info.axes.size();
	int samples = info.samples;

	MolRefiner *mr = new MolRefiner(_map, &info, samples, dims);
	mr->setCluster(cluster);
	mr->nudgeAxis();
	mr->startCalculator();

	std::vector<ResidueTorsion> list = cluster->dataGroup()->headers();

	for (size_t i = 0; i < info.axes.size(); i++)
	{
		std::vector<Angular> convert;
		const Refine::Axis &axis = info.axes[i];
		for (size_t j = 0; j < axis.angles.size(); j++)
		{
			float angle = axis.angles[j].angle;
			convert.push_back(Angular(angle / 2.f));
		}
		
		mr->supplyTorsions(list, convert);
	}

	_molRefiners[mol] = mr;
}

void Refinement::play()
{
	for (Refine::Info &info  : _molDetails)
	{
		MolRefiner *mr = _molRefiners[info.molecule];
		mr->runEngine();
	}
	
	_model->write("after_refinement.pdb");
}

ArbitraryMap *Refinement::calculatedMapAtoms()
{
	return nullptr;
}

float Refinement::comparisonWithData()
{
	ArbitraryMap *calc = calculatedMapAtoms();
	float result = _model->comparisonWithData(calc);
	std::cout << "Comparison result: " << result << std::endl;
	return result;
}
