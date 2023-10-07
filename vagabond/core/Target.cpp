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

#include "Target.h"
#include <vagabond/utils/Hypersphere.h>
#include "RopeCluster.h"
#include "Torsion2Atomic.h"

void Target::minMaxComponents(size_t num_axes, std::vector<float> &mins, 
                              std::vector<float> &maxes)
{
	mins = std::vector<float>(num_axes, FLT_MAX);
	maxes = std::vector<float>(num_axes, -FLT_MAX);
	MetadataGroup *grp = _tCluster->dataGroup();

	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		std::vector<float> entry = _tCluster->mappedVector(i);
		Floats truncated(entry);
		truncated.resize(num_axes);
		
		for (size_t j = 0; j < truncated.size(); j++)
		{
			mins[j] = std::min(mins[j], truncated[j]);
			maxes[j] = std::max(maxes[j], truncated[j]);
		}
	}

	int idx = grp->indexOfObject(_reference);
	std::vector<float> origin = _tCluster->mappedVector(idx);

	for (size_t j = 0; j < mins.size(); j++)
	{
		mins[j] -= origin[j];
		maxes[j] -= origin[j];
		std::cout << mins[j] << " to " << maxes[j] << std::endl;
	}
}

Target::Target(int num_axes, TorsionCluster *cluster, Instance *ref)
{
	_reference = ref;
	_nAxes = num_axes;
	_weights = std::vector<float>(_nAxes, 0.5);
	_tCluster = cluster;
	
	transformMatrix();
}

Target::~Target()
{
	freeMatrix(&_shift);
}

void Target::transformMatrix()
{
	std::vector<float> mins, maxes;
	minMaxComponents(_nAxes, mins, maxes);

	setupMatrix(&_shift, _nAxes + 1, _nAxes + 1);
	
	_shift[_nAxes][_nAxes] = 1;
	for (size_t i = 0; i < _nAxes; i++)
	{
		// translation down final column to centre
		_shift[i][i] = (maxes[i] - mins[i]) / 8;
	}
	
	printMatrix(&_shift);
}

const std::vector<Floats> &Target::pointsForScore()
{
	_points.clear();
	
	Hypersphere sphere(_nAxes, 200);
	sphere.prepareFibonacci();

	for (float j = 0; j < sphere.count(); j++)
	{
		std::vector<float> p = sphere.point(j);
		
		for (size_t k = 0; k < p.size() && k < _weights.size(); k++)
		{
			p[k] *= _weights[k];
		}
		
		transformCoordinates(p);
		Floats fs(p);
		fs *= 1.0;
		_points.push_back(fs);
	}
	
	return _points;
}

void Target::transformCoordinates(std::vector<float> &coord)
{
	coord.resize(_nAxes);
	coord.push_back(1);
	std::vector<float> result(coord.size());
	multMatrix(_shift, &coord[0], &result[0]);
	coord = result;
	coord.pop_back();
}
