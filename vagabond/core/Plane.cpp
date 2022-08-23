// 
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

#include "Plane.h"
#include <vagabond/utils/FileReader.h>
#include "MetadataGroup.h"
#include "ForceField.h"
#include "Molecule.h"

Plane::Plane(Molecule *mol, Cluster<MetadataGroup> *cluster)
: StructureModification(mol, 1, 2)
{
	_cluster = cluster;
	_molecule->model()->load();
	AtomContent *grp = _molecule->model()->currentAtoms();
	_fullAtoms = grp;
	startCalculator();
}

void Plane::addAxis(std::vector<ResidueTorsion> &list, std::vector<float> &values,
	             std::vector<float> &mapped)
{
	if (_axis >= 2)
	{
		throw std::runtime_error("Too many axes in plane");
	}


	_torsions[_axis] = values;
	_cluster->dataGroup()->applyNormals(_torsions[_axis]);

	_axes[_axis] = mapped;

	supplyTorsions(list, values);
}

void Plane::refresh()
{
	_scores.clear();
	_scoreMap.clear();
	_points.clear();
	_toIndex.clear();

	for (int j = -_counts[0]; j < _counts[1]; j++)
	{
		for (int i = -_counts[0]; i < _counts[0]; i++)
		{
			float x = (float)i * _scale;
			float y = (float)j * _scale;
			_toIndex[i][j] = _points.size();
			if (i == 0 && j == 0)
			{
				_refIdx = _points.size();
			}

			submitJob(x, y);
			_points.push_back(glm::vec3(x, y, 0));
			_scores.push_back(0);
		}
	}
	
	collectResults();
	reportScores();
}

void Plane::reportScores()
{

}

void Plane::submitJob(float x, float y)
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		job.custom.vecs[0].mean[0] = x;
		job.custom.vecs[0].mean[1] = y;
		job.requests = static_cast<JobType>(JobScoreStructure);
		int job_num = calc->submitJob(job);
		_scoreMap[job_num] = _scores.size();
	}
}

void Plane::collectResults()
{
	bool more = true;
	while (more)
	{
		more = false;
		for (BondCalculator *calc : _calculators)
		{
			Result *r = calc->acquireResult();

			if (r == nullptr)
			{
				continue;
			}

			more = true;

			if (r->requests & JobScoreStructure)
			{
				int num = r->ticket;
				
				if (_scoreMap.count(num))
				{
					int idx = _scoreMap[num];
					_scores[idx] += r->score;
				}
			}
		}
	}

	_scoreMap.clear();
	
	CorrelData cd = empty_CD();
	for (size_t i = 0; i < _scores.size(); i++)
	{
		add_to_CD(&cd, _scores[i], 0.f);
	}
	
	double mean, stdev;
	mean_stdev_CD(cd, &mean, &stdev);
	
	_mean = mean;
	_stdev = stdev;
}

glm::vec3 Plane::generateVertex(int i, int j)
{
	i -= _counts[0];
	j -= _counts[1];

	MetadataGroup *group = _cluster->dataGroup();
	int idx = group->indexOfObject(_molecule);
	std::vector<float> mol = group->differenceVector(idx);

	int pdx = _toIndex[i][j];
	float mx = _points[pdx].x;
	float my = _points[pdx].y;

	for (size_t i = 0; i < mol.size(); i++)
	{
		float add = mx * _torsions[0][i] + my * _torsions[1][i];
		mol[i] += add;
	}

//	_cluster->mapVector(mol);

	glm::vec3 pos = _cluster->point(idx);
	glm::vec3 horz = glm::vec3(_axes[0][0], _axes[0][1], _axes[0][2]);
	glm::vec3 vert = glm::vec3(_axes[1][0], _axes[1][1], _axes[1][2]);
	
	horz *= mx;
	vert *= my;

	pos += horz;
	pos += vert;
	return pos;
}

void Plane::setupForceField()
{
	if (_forceField != nullptr)
	{
		delete _forceField;
		_forceField = nullptr;
	}

	_molecule->model()->load();
	_forceField = new ForceField(_molecule->currentAtoms());
	_forceField->setTemplate(ForceField::CAlphaSeparation);
	_forceField->setup();
	
	for (BondCalculator *calc : _calculators)
	{
		calc->setForceField(_forceField);
	}
}
