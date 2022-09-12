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

Route::Route(Molecule *mol, int dims) : StructureModification(mol, 1, dims)
{

}

void Route::supplyAxes(const std::vector<ResidueTorsion> &list)
{
	// basically same as a simple basis, massive identity matrix
	std::vector<float> vals(list.size(), 0);

	for (int i = 0; i < list.size(); i++)
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
	for (BondCalculator *calc : _calculators)
	{
		if (!(calc->pipelineType() & BondCalculator::PipelineForceField))
		{
			continue;
		}

		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		for (size_t i = 0; i < _dims; i++)
		{
			job.custom.vecs[0].mean[i] = _points[idx][i];
		}

		job.requests = static_cast<JobType>(JobExtractPositions);
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
			r->transplantLastPosition();
		}
	}
	
	return;
}

