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
	_mask.clear();
	startCalculator();

	std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();

	std::vector<float> vals(1, 1);
	int count = 0;
	for (size_t i = 0; i < _dims; i++)
	{
		std::vector<ResidueTorsion> single(1, list[i]);
		bool mask = supplyTorsions(single, vals);
		_mask.push_back(mask);
		count += (mask ? 1 : 0);
	}
	
	std::cout << "Using " << count << " torsions for route out of "
	<< _dims << " total." << std::endl;
}

void Route::addPoint(Point &values)
{
	_points.push_back(values);
}

void Route::addEmptyPoint()
{
	_points.push_back(Point(_dims, 0));
}

void Route::clearPoints()
{
	_points.clear();
}

void Route::submitJob(int idx)
{
	if ((idx > 0 && idx >= _points.size()) || idx < 0)
	{
		return;
	}

	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);

		for (size_t i = 0; i < _dims; i++)
		{
			float value = 0;
			if (_points.size() > 0)
			{
				value = _points[idx][i];
			}
			job.custom.vecs[0].mean[i] = value;
		}

		job.requests = static_cast<JobType>(JobExtractPositions |
		                                    JobCalculateDeviations);

		calc->submitJob(job);
	}

	_score = 0;
	double n = 0;
	for (BondCalculator *calc : _calculators)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			return;
		}

		if (r->requests & JobExtractPositions)
		{
			r->transplantLastPosition();
		}
		
		if (r->requests & JobCalculateDeviations)
		{
			if (r->deviation == r->deviation)
			{
				_score += r->deviation;
				n++;
			}
		}
	}
	
	_score /= n;
	if (_score != _score)
	{
		_score = 0;
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

