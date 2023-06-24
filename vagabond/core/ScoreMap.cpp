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

#include "ScoreMap.h"
#include <vagabond/core/SpecificNetwork.h>
#include <vagabond/utils/Mapping.h>

ScoreMap::ScoreMap(Mapped<float> *mapped, SpecificNetwork *specified)
{
	_specified = specified;
	_mapped = mapped;
}

ScoreMap::~ScoreMap()
{
}

void ScoreMap::sendObject(std::string tag, void *object)
{
	if (tag == "atom_list")
	{
		AtomPosList *apl = static_cast<AtomPosList *>(object);
		_comparer.process(*apl);
	}
}

float ScoreMap::submitJobs(const Points &points, bool make_aps)
{
	struct TicketPoint
	{
		int ticket;
		std::vector<float> point;
	};

	std::vector<TicketPoint> tickets;
	int save_id = 0;
	for (const std::vector<float> &p : points)
	{
		int ticket = _specified->submitJob(make_aps, p, save_id);
		tickets.push_back({ticket, p});
		save_id++;
	}
	
	_specified->setResponder(this);
	_specified->retrieve();
	_specified->removeResponder(this);
	
	float total = 0; float count = 0;
	for (TicketPoint &tp : tickets)
	{
		float sc = _specified->deviation(tp.ticket);
		total += sc; count++;
		
		if (_eachHandler)
		{
			_eachHandler(sc, tp.point);
		}
	}
	
	total /= count;

	return total;
}

float ScoreMap::scoreForPoints(const Points &points)
{
	if (_mode == Unset)
	{
		throw std::runtime_error("Score mode is not set");
	}

	float value = processPoints(points, modeNeedsAPS(_mode));

	if (_mode == Distance || _mode == AssessSplits)
	{
		value = _comparer.quickScore();
	}
	
	return value;
}

float ScoreMap::processPoints(const Points &points, bool make_aps)
{
	float basic_score = submitJobs(points, make_aps);
	return basic_score;
}


void ScoreMap::setFilters(AtomFilter &left, AtomFilter &right)
{
	_comparer.setLeftFilter(left);
	_comparer.setRightFilter(right);
}

bool ScoreMap::hasMatrix()
{
	return _comparer.receivedCount() > 0;
}

std::vector<Atom *> ScoreMap::atoms()
{
	return _comparer.leftAtoms();
}

PCA::Matrix ScoreMap::matrix()
{
	return _comparer.matrix();
}
