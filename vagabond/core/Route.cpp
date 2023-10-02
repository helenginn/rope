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
#include "Polymer.h"
#include "Grapher.h"
#include "TorsionBasis.h"
#include "MetadataGroup.h"
#include "RopeCluster.h"

Route::Route(Instance *inst, TorsionCluster *cluster, int dims) 
: StructureModification(inst, 1, dims)
{
	_cluster = cluster;
	useForceField(true);
	_torsionType = TorsionBasis::TypeSimple;
	instance()->load();
}

Route::~Route()
{
	instance()->unload();
}

void Route::setup()
{
	if (_rawDest.size() == 0 && motionCount() == 0)
	{
		throw std::runtime_error("No destination or prior set for route");
	}

	_fullAtoms = _instance->currentAtoms();
	startCalculator();
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

float Route::submitJobAndRetrieve(int idx, bool show, bool forces)
{
	_point2Score.clear();
	_ticket2Point.clear();

	submitJob(idx, show, forces);
	retrieve();
	
	return _point2Score[idx].scores;
}

void Route::submitJob(int idx, bool show, bool forces)
{
	if ((idx > 0 && idx >= _points.size()) || idx < 0)
	{
		return;
	}
	
	if (forces)
	{
		show = true;
	}
	
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		int dims = _calc2Destination[calc].size();
		job.custom.allocate_vectors(1, dims, _num);
		job.fraction = idx / (float)(pointCount() - 1);
		job.parameters.resize(dims);

		for (size_t i = 0; i < dims; i++)
		{
			float value = 0;
			int calc_idx = _calc2Destination[calc][i];
			
			if (calc_idx < 0) continue;
			if (_points.size() > idx && _points[idx].size() > calc_idx)
			{
				value = _points[idx][calc_idx];
			}
			job.parameters[i] = value;
		}

		job.requests = static_cast<JobType>(JobPositionVector |
		                                    JobCalculateDeviations);
		if (!show)
		{
			job.requests = JobCalculateDeviations;
		}
		
		if (forces && _pType & BondCalculator::PipelineForceField)
		{
			job.requests = (JobType)(JobScoreStructure |
			                         job.requests);
		}

		int t = calc->submitJob(job);
		_ticket2Point[t] = idx;
	}

	_point2Score[idx] = Score{};
}

void Route::useForceField(bool use)
{
	if (!use)
	{
		_pType = BondCalculator::PipelineAtomPositions;

	}
	else
	{
		_pType = BondCalculator::PipelineForceField;
	}
}

void Route::customModifications(BondCalculator *calc, bool has_mol)
{
	if (!has_mol)
	{
		return;
	}

	calc->setPipelineType(_pType);
	
	if (_pType & BondCalculator::PipelineForceField)
	{
		FFProperties props;
		props.group = _instance->currentAtoms();
		props.t = FFProperties::VdWContacts;
		calc->setForceFieldProperties(props);
	}

	calc->setSampler(nullptr);
	calc->manager().setDefaultCoordTransform(JobManager::identityTransform());
}

const Grapher &Route::grapher() const
{
	const Grapher &g = _calculators[_grapherIdx]->grapher();
	return g;
}

bool Route::incrementToAtomGraph(AtomGraph *ag)
{
	AtomGraph *comp = nullptr;
	do
	{
		comp = grapher().graph(ag->atom);
	}
	while (!comp && incrementGrapher());
	
	return (comp != nullptr);
}

AtomGraph *Route::grapherForTorsionIndex(int idx)
{
	AtomGraph *ag = nullptr;
	do
	{
		ag = grapher().graph(parameter(idx));
	}
	while (!ag && incrementGrapher());

	return ag;
}

bool Route::incrementGrapher()
{
	_grapherIdx++;
	if (_grapherIdx >= _calculators.size())
	{
		_grapherIdx = 0;
		return false;
	}
	
	return true;
}

void Route::setFlips(std::vector<int> &idxs, std::vector<int> &fs)
{
	for (size_t j = 0; j < idxs.size(); j++)
	{
		setFlip(idxs[j], fs[j]);
	}
}


void Route::bringTorsionsToRange()
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		while (destination(i) >= 180)
		{
			destination(i) -= 360;
		}
		while (destination(i) < -180)
		{
			destination(i) += 360;
		}
	}
}

void Route::reportFound()
{
	return;
	for (size_t i = 0; i < _missing.size(); i++)
	{
		std::cout << "Missing torsion " << _missing[i]->desc() << " for "
		<< "residue " << _missing[i]->residueId().str() << std::endl;
	}

}

void Route::getParametersFromBasis()
{
	if (_motions.size() > 0)
	{
		std::cout << "skipping" << std::endl;
		return;
	}

	_missing.clear();

	std::vector<ResidueTorsion> headers = _cluster->dataGroup()->headers();
	RTAngles list = RTAngles::angles_from(headers, {});

	std::vector<Motion> motions;
	std::vector<ResidueTorsion> torsions;

	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->torsionBasis();
		
		for (size_t i = 0; i < basis->parameterCount(); i++)
		{
			Parameter *p = basis->parameter(i);
			int idx = _instance->indexForParameterFromList(p, list);
			
			if (idx < 0)
			{
				_missing.push_back(p);
				continue;
			}

			ResidueTorsion rt = list.c_rt(idx);
			rt.attachToInstance(_instance);
			float final_angle = _rawDest[idx];

			torsions.push_back(rt);
			motions.push_back(Motion{WayPoints(), false, final_angle});
		}
	}

	_motions = RTMotion::motions_from(torsions, motions);
	connectParametersToDestination();
}

void Route::connectParametersToDestination()
{
	_calc2Destination.clear();

	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->torsionBasis();

		for (size_t i = 0; i < basis->parameterCount(); i++)
		{
			Parameter *p = basis->parameter(i);
			
			int chosen = -1;
			for (int j = 0; j < motionCount(); j++)
			{
				if (parameter(j) == p)
				{
					chosen = j;
					break;
				}
			}
			
			_calc2Destination[calc].push_back(chosen);
		}
	}
}

void Route::prepareDestination()
{
	if (_cluster == nullptr)
	{
		return;
	}
	
	if (_rawDest.size() == 0)
	{
		throw std::runtime_error("Raw destination not set, trying to prepare"\
		                         "destination");
	}
	
	getParametersFromBasis();
	reportFound();
}

int Route::indexOfParameter(Parameter *t)
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (parameter(i) == t)
		{
			return i;
		}
	}
	
	return -1;
}

void Route::printWayPoints()
{
	std::cout << "Waypoints:" << std::endl;
	for (size_t i = 0; i < wayPointCount(); i++)
	{
		std::cout << parameter(i)->desc() << " ";
		std::cout << " flip: " << (flip(i) ? "yes" : "no") << "; ";
		for (size_t j = 0; j < wayPointCount(); j++)
		{
			std::cout << "(" << wayPoints(i)[j].progress() << ", ";
			std::cout << wayPoints(i)[j].fraction() << ") ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

float Route::getTorsionAngle(int i)
{
	if (!flip(i))
	{
		return destination(i);
	}
	
	if (destination(i) > 0)
	{
		return destination(i) - 360;
	}
	else 
	{
		return destination(i) + 360;
	}
}

std::vector<ResidueTorsion> Route::residueTorsions()
{
	std::vector<ResidueTorsion> rts;
	for (size_t i = 0; i < motionCount(); i++)
	{
		rts.push_back(residueTorsion(i));
	}

	return rts;
}
