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

#define _USE_MATH_DEFINES
#include <math.h>
#include "Plane.h"
#include <vagabond/utils/FileReader.h>
#include "MetadataGroup.h"
#include "ForceFieldHandler.h"
#include "Molecule.h"
#include <thread>

Plane::Plane(Molecule *mol, Cluster<MetadataGroup> *cluster)
: StructureModification(mol, 1, 2)
{
	_threads = 4;
	_cluster = cluster;
	_molecule->model()->load();
	_pType = BondCalculator::PipelineForceField;
	AtomContent *grp = _molecule->model()->currentAtoms();
	_fullAtoms = grp;
	startCalculator();

//	_scale /= _cluster->scaleFactor();
}

Plane::~Plane()
{
	_molecule->model()->unload();
	cancelRun();
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
	
	double sc = _cluster->scaleFactor();

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

			submitJob(x * sc, y * sc);
			_points.push_back(glm::vec3(x, y, 0));
			_scores.push_back(0);
		}
	}
	
	cancelRun();
	cleanupRun();
	_worker = new std::thread(&Plane::backgroundCollection, this);
}

void Plane::submitJob(float x, float y)
{
	for (BondCalculator *calc : _calculators)
	{
		if (!(calc->pipelineType() & BondCalculator::PipelineForceField))
		{
			continue;
		}
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		job.custom.vecs[0].mean[0] = x;
		job.custom.vecs[0].mean[1] = y;
		job.requests = static_cast<JobType>(JobScoreStructure | JobExtractPositions);
		int job_num = calc->submitJob(job);
		_scoreMap[job_num] = _scores.size();
	}
}

void Plane::collectResults()
{
	int count = 0;
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

			int num = r->ticket;
			if (r->requests & JobScoreStructure)
			{
				
				if (_scoreMap.count(num))
				{
					int idx = _scoreMap[num];
					_scores[idx] += r->score;
				}
			}

//			if (r->requests & JobExtractPositions)
			{
//				r->transplantLastPosition();
//				_molecule->model()->write("set/point_" + std::to_string(num) + ".pdb");

			}
			
			r->destroy();
			
			count++;
			if (count % 10 == 0)
			{
				triggerResponse();
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

	glm::vec3 pos = _cluster->point(idx);
	glm::vec3 horz = glm::vec3(_axes[0][0], _axes[0][1], _axes[0][2]);
	glm::vec3 vert = glm::vec3(_axes[1][0], _axes[1][1], _axes[1][2]);
	
	horz *= mx;
	vert *= my;

	pos += horz;
	pos += vert;
	return pos;
}

void Plane::customModifications(BondCalculator *calc, bool has_mol)
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

void Plane::cancelRun()
{
	_finish = true;
}

void Plane::cleanupRun()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

}
