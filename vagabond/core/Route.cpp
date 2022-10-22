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
#include "BondSequence.h"
#include "MetadataGroup.h"
#include <vagabond/c4x/Cluster.h>

Route::Route(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims) 
: StructureModification(mol, 1, dims)
{
	_cluster = cluster;
	_pType = BondCalculator::PipelineForceField;
	_torsionType = TorsionBasis::TypeSimple;
}

void Route::setup()
{
	if (_rawDest.size() == 0 && destinationSize() == 0)
	{
		throw std::runtime_error("No destination set for route");
	}

	_fullAtoms = _molecule->currentAtoms();
	_mask.clear();
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
		job.custom.allocate_vectors(1, _dims, _num);
		job.fraction = idx / (float)(pointCount() - 1);

		for (size_t i = 0; i < _dims; i++)
		{
			float value = 0;
			if (_points.size() > 0 && _points[idx].size() > i)
			{
				value = _points[idx][i];
			}
			job.custom.vecs[0].mean[i] = value;
		}

		job.requests = static_cast<JobType>(JobExtractPositions |
		                                    JobCalculateDeviations);
		if (!show)
		{
			job.requests = JobCalculateDeviations;
		}
		
		if (forces)
		{
			job.requests = (JobType)(JobScoreStructure | job.requests);
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

			found = true;
			if (r->requests & JobExtractPositions)
			{
				r->transplantLastPosition();
				
				for (AtomPosMap::iterator it = r->aps.begin(); 
				     it != r->aps.end(); it++)
				{
					Atom *atom = it->first;
				}
			}
			
			if (r->requests & JobScoreStructure)
			{
				r->transplantColours();
			}

			if (r->requests & JobCalculateDeviations)
			{
				int t = r->ticket;
				int idx = _ticket2Point[t];
				if (r->deviation == r->deviation)
				{
					_point2Score[idx].scores += r->deviation;
					_point2Score[idx].divs++;
				}
			}
			
			r->destroy();
		}
	}
	
	for (TicketScores::iterator it = _point2Score.begin();
	     it != _point2Score.end(); it++)
	{
		it->second.scores /= it->second.divs;
		it->second.divs = 1;
	}
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
	props.t = FFProperties::VdWContacts;
	calc->setForceFieldProperties(props);
}

const Grapher &Route::grapher() const
{
	const BondSequence *seq = _calculators[_grapherIdx]->sequence();
	const Grapher &g = seq->grapher();

	return g;
}

bool Route::incrementGrapher()
{
	_grapherIdx++;
	if (_grapherIdx >= _calculators.size())
	{
		_grapherIdx = 0;
		std::cout << "Grapher index now: " << _grapherIdx << std::endl;
		return false;
	}
	
	std::cout << "Grapher index now: " << _grapherIdx << std::endl;
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
	_mask = std::vector<bool>(destinationSize(), true);
	
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

	const std::vector<ResidueTorsion> &list = _cluster->dataGroup()->headers();
	std::vector<bool> found(list.size(), false);

	_torsions.clear();
	_destination.clear();

	int count = 0;
	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->sequence()->torsionBasis();
		
		for (size_t i = 0; i < basis->torsionCount(); i++)
		{
			BondTorsion *t = basis->torsion(i);
			float v = _molecule->valueForTorsionFromList(t, list, _rawDest, found);
			if (v != v)
			{
				v = 0;
			}

			_torsions.push_back(t);
			_destination.push_back(v);
			
			/* each calculator will only be sensitive to a subset of our
			 * destination, so we need to keep records */
			_calc2Destination[calc].push_back(count);
			count++;
		}
	}
	
	populateWaypoints();
}
