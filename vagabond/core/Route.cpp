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
#include <vagabond/c4x/Cluster.h>

Route::Route(Instance *inst, Cluster<MetadataGroup> *cluster, int dims) 
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

	if (_rawDest.size() == 0 && destinationSize() == 0)
	{
		throw std::runtime_error("No destination set for route");
	}

	_fullAtoms = _instance->currentAtoms();
	startCalculator();
	connectParametersToDestination();
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

		for (size_t i = 0; i < dims; i++)
		{
			float value = 0;
			int calc_idx = _calc2Destination[calc][i];
			
			if (calc_idx < 0) continue;
			if (_points.size() > idx && _points[idx].size() > calc_idx)
			{
				value = _points[idx][calc_idx];
			}
			job.custom.vecs[0].mean[i] = value;
		}

		job.requests = static_cast<JobType>(JobExtractPositions |
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

void Route::retrieve()
{
	bool found = true;

	while (found)
	{
		found = false;

		for (BondCalculator *calc : _calculators)
		{
			Result *r = calc->acquireResult();

			if (r == nullptr)
			{
				continue;
			}

			int t = r->ticket;
			int idx = _ticket2Point[t];

			found = true;
			if (r->requests & JobExtractPositions)
			{
				r->transplantPositions();
			}
			if (r->requests & JobSolventSurfaceArea)
			{
				std::cout << r->surface_area << std::endl;
			}
			
			if (r->requests & JobScoreStructure)
			{
				r->transplantColours();
				
				if (r->score == r->score)
				{
					_point2Score[idx].scores += r->score;
					_point2Score[idx].sc_num++;
				}
			}

			if (r->requests & JobCalculateDeviations)
			{
				if (r->deviation == r->deviation)
				{
					_point2Score[idx].deviations += r->deviation;
					_point2Score[idx].divs++;
				}
			}
			
			r->destroy();
		}
	}
	
	for (TicketScores::iterator it = _point2Score.begin();
	     it != _point2Score.end(); it++)
	{
		it->second.scores /= it->second.sc_num;
		it->second.deviations /= it->second.divs;
		it->second.divs = 1;
		it->second.sc_num = 1;
	}
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

void Route::setFlips(std::vector<int> &idxs, std::vector<bool> &fs)
{
	for (size_t j = 0; j < idxs.size(); j++)
	{
		setFlip(idxs[j], fs[j]);
	}
}


void Route::bringTorsionsToRange()
{
	for (size_t i = 0; i < destinationSize(); i++)
	{
		while (destination(i) >= 180)
		{
			_destination[i] -= 360;
		}
		while (destination(i) < -180)
		{
			_destination[i] += 360;
		}
	}
}

void Route::clearWayPointFlips()
{
	_wayPoints.clear();
	_flips.clear();
}

void Route::populateWaypoints()
{
	clearWayPointFlips();
	
	for (size_t i = 0; i < destinationSize(); i++)
	{
		WayPoints wp;
		wp.push_back(WayPoint::startPoint());
		_flips.push_back(false);

		if (fabs(destination(i)) < 30)
		{
			wp.push_back(WayPoint(0.33, 0.33));
			wp.push_back(WayPoint(0.66, 0.66));
		}
		else
		{
			wp.push_back(WayPoint::midPoint());
		}

		wp.push_back(WayPoint::endPoint());

		_wayPoints[i] = wp;
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
	_parameters.clear();
	_missing.clear();
	_destination.clear();

	const std::vector<ResidueTorsion> &list = _cluster->dataGroup()->headers();
	std::vector<bool> found(list.size(), false);

	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->torsionBasis();
		
		for (size_t i = 0; i < basis->parameterCount(); i++)
		{
			Parameter *p = basis->parameter(i);
			int idx = _instance->indexForParameterFromList(p, list);
			float v = 0;
			
			if (idx < 0)
			{
				_missing.push_back(p);
				continue;
			}
			else
			{
				v = _rawDest[idx];
			}
			
			const ResidueTorsion &rt = list[idx];

			addParameter(rt, p);
			_destination.push_back(v);
		}
	}
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
			for (int j = 0; j < _parameters.size(); j++)
			{
				if (_parameters[j].param == p)
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
	connectParametersToDestination();
	reportFound();
	populateWaypoints();
}

int Route::indexOfTorsion(BondTorsion *t)
{
	for (size_t i = 0; i < parameterCount(); i++)
	{
		if (parameter(i) == t)
		{
			return i;
		}
	}
	
	return -1;
}

void Route::extractWayPoints(Route *other)
{
	if (_calculators.size() == 0)
	{
		return;
	}

	const Grapher &g = grapher();
	const Grapher &h = other->grapher();

	for (size_t i = 0; i < g.graphCount(); i++)
	{
		AtomGraph *gr = g.graph(i);
		
		if (gr->torsion == nullptr)
		{
			continue;
		}

		for (size_t j = 0; j < h.graphCount(); j++)
		{
			AtomGraph *hr = h.graph(j);

			if (hr->torsion == nullptr || gr->torsion != hr->torsion)
			{
				continue;
			}
			
			if (gr->atom != hr->atom)
			{
				continue;
			}
			
			int src = other->indexOfTorsion(hr->torsion);
			int dest = indexOfTorsion(hr->torsion);
			
			if (src < 0 || dest < 0)
			{
				continue;
			}
			
			const WayPoints &wps = other->wayPoints(src);
			setWayPoints(dest, wps);
			bool flip = other->flip(src);
			setFlip(dest, flip);
		}
	}
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

void Route::setDestination(Point &d)
{
	_destination = d;
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

std::vector<ResidueTorsion> Route::residueTorsions() const
{
	std::vector<ResidueTorsion> rts;
	for (size_t i = 0; i < parameterCount(); i++)
	{
		rts.push_back(residueTorsion(i));
	}

	return rts;
}
