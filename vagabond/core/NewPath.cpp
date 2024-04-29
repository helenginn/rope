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

#include "NewPath.h"
#include "PlausibleRoute.h"
#include "Path.h"
#include "Entity.h"

NewPath::NewPath(Instance *from, Instance *to, Path *blueprint)
{
	addLinkedInstances(from, to);
	_blueprint = blueprint;
}

NewPath::NewPath(Path *blueprint)
{
	_blueprint = blueprint;
}

TorsionData *NewPath::addPair(Instance *from, Instance *to)
{
	std::vector<ResidueTorsion> headers = from->residueTorsionList();
	size_t num = headers.size();
	TorsionData *prep = new TorsionData(num);
	prep->addHeaders(headers);

	from->addTorsionsToGroup(*prep, rope::RefinedTorsions);
	to->addTorsionsToGroup(*prep, rope::RefinedTorsions);

	return prep;
}

TorsionData *NewPath::dataAllPairs()
{
	TorsionData *all = nullptr;
	
	for (const InstancePair &pair : _pairs)
	{
		TorsionData *next = addPair(pair.start, pair.end);
		std::cout << pair.start->id() << " to " << pair.end->id() << " gives "
		<< next->headerCount() << " headers." << std::endl;
		if (all == nullptr)
		{
			all = next;
		}
		else
		{
			TorsionData *combined = new TorsionData(*all + *next);
			delete all;
			delete next;
			all = combined;
		}
	}

	return all;
}

PlausibleRoute *NewPath::operator()()
{
	TorsionData *prep = dataAllPairs();

	RTAngles list = prep->emptyAngles(true);
	
	std::vector<Angular> from_angles, to_angles;
	from_angles = prep->vector(0);
	to_angles = prep->vector(1);

	for (size_t i = 0; i < to_angles.size(); i++)
	{
		list.storage(i) = to_angles[i] - from_angles[i];
	}

	PlausibleRoute *pr = new PlausibleRoute(list);

	for (const InstancePair &pair : _pairs)
	{
		pr->addLinkedInstances(pair.start, pair.end);
	}
	
	if (_blueprint)
	{
		RTMotion motions = _blueprint->motions();
		RTPeptideTwist twists = _blueprint->twists();
		motions.attachInstance(_from);
		twists.attachInstance(_from);
		pr->setMotions(motions);
		pr->setTwists(twists);
		pr->setNew(false);
	}
	
	delete prep;

	return pr;
}


void NewPath::addLinkedInstances(Instance *from, Instance *to)
{
	if (from->hasSequence() && false)
	{
		_pairs.insert(_pairs.begin(), {from, to});
	}
	else
	{
		_pairs.push_back({from, to});
	}
	if (!_from)
	{
		_from = from;
		_to = to;
	}
}
