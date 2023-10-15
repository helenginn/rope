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

Route::Route(Instance *from, Instance *to, const RTAngles &list)
: StructureModification(from, 1, list.size())
{
	_endInstance = to;
	_source = list;
	_torsionType = TorsionBasis::TypeSimple;
	instance()->load();
}

Route::~Route()
{
	instance()->unload();
}

void Route::setup()
{
	if (_source.size() == 0 && motionCount() == 0)
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

	float fraction = idx / (float)(pointCount() - 1);
	
	BondCalculator *calc = calculator();

	Job job{};
	job.parameters = _points[idx];
	job.atomTargets = AtomBlock::prepareMovingTargets(calc, fraction);

	job.requests = static_cast<JobType>(JobPositionVector |
	                                    JobCalculateDeviations);
	if (!show)
	{
		job.requests = JobCalculateDeviations;
	}

	int t = calc->submitJob(job);
	_ticket2Point[t] = idx;

	_point2Score[idx] = Score{};
}

void Route::customModifications(BondCalculator *calc, bool has_mol)
{
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

BondCalculator *Route::calculator()
{
	return _instanceToCalculator[_instance];
}

void Route::getParametersFromBasis()
{
	if (_motions.size() > 0)
	{
		std::cout << "skipping pick up of parameters, done already" << std::endl;
		return;
	}

	_missing.clear();

	std::vector<Motion> tmp_motions;
	std::vector<ResidueTorsion> torsions;

	BondCalculator *calc = calculator();
	TorsionBasis *basis = calc->torsionBasis();

	for (size_t i = 0; i < basis->parameterCount(); i++)
	{
		Parameter *p = basis->parameter(i);
		int idx = _instance->indexForParameterFromList(p, _source);

		if (idx < 0)
		{
			torsions.push_back(ResidueTorsion{});
			tmp_motions.push_back(Motion{WayPoints(), false, 0});
			_missing.push_back(p);
			continue;
		}

		ResidueTorsion rt = _source.c_rt(idx);
		rt.attachToInstance(_instance);
		float final_angle = _source.storage(idx);
		
		if (final_angle != final_angle) 
		{
			final_angle = 0;
			std::cout << "WARNING!" << final_angle << std::endl;
		}

		torsions.push_back(rt);
		tmp_motions.push_back(Motion{WayPoints(), false, final_angle});
	}

	_motions = RTMotion::motions_from(torsions, tmp_motions);
	bringTorsionsToRange();
}

void Route::prepareDestination()
{
	if (_source.size() == 0 && _motions.size() == 0)
	{
		throw std::runtime_error("Raw destination not set, nor motions for "\
		                         "destination");
	}
	
	getParametersFromBasis();
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
