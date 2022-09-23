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

#include "Route.h"
#include "Molecule.h"
#include "MetadataGroup.h"
#include <vagabond/c4x/Cluster.h>

Route::Route(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims) 
: StructureModification(mol, 1, dims)
{
	_cluster = cluster;
	_pType = BondCalculator::PipelineForceField;

}

void Route::setup()
{
	_fullAtoms = _molecule->currentAtoms();
	startCalculator();

	std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();

	std::vector<float> vals(_dims, 0);
	for (size_t i = 0; i < _dims; i++)
	{
		if (i > 0)
		{
			vals[i - 1] = 0;
		}
		vals[i] = 1;
		supplyTorsions(list, vals);
	}
}

void Route::addPoint(Point &values)
{
	_points.push_back(values);
}

void Route::submitJob(int idx)
{
	if (idx >= _points.size() || idx < 0)
	{
		return;
	}

	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);

		for (size_t i = 0; i < _dims; i++)
		{
			job.custom.vecs[0].mean[i] = _points[idx][i];
		}

		job.requests = static_cast<JobType>(JobExtractPositions |
		                                    JobScoreStructure);
		calc->submitJob(job);
	}

	for (BondCalculator *calc : _calculators)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			return;
		}

		if (r->requests & JobExtractPositions)
		{
			std::cout << "Grabbing positions" << std::endl;
			r->transplantLastPosition();
		}

		if (r->requests & JobScoreStructure)
		{
			std::cout << r->score << std::endl;

		}
	}
	
	return;
}

void Route::customModifications(BondCalculator *calc, bool has_mol)
{
	if (!has_mol)
	{
		return;
	}

	calc->setPipelineType(_pType);
	FFProperties props;
	props.group = _molecule->currentAtoms();
	props.t = FFProperties::CAlphaSeparation;
	calc->setForceFieldProperties(props);
}

