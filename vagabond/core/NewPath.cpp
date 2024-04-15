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
	_from = from;
	_to = to;
	_blueprint = blueprint;
}

PlausibleRoute *NewPath::operator()()
{
	Entity *entity = _from->entity();
	std::vector<Instance *> instances = entity->instances();

	Polymer *pFrom = static_cast<Polymer *>(_from);
	std::vector<ResidueTorsion> headers;
	pFrom->sequence()->addResidueTorsions(headers, false);
	for (ResidueTorsion &rt : headers)
	{
		rt.attachToInstance(_from);
	}

	size_t num = headers.size();
	TorsionData prep(num);
	prep.addHeaders(headers);

	_from->addTorsionsToGroup(prep, rope::RefinedTorsions);
	_to->addTorsionsToGroup(prep, rope::RefinedTorsions);

	RTAngles list = prep.emptyAngles(true);
	
	std::vector<Angular> from_angles, to_angles;
	from_angles = prep.vector(0);
	to_angles = prep.vector(1);

	for (size_t i = 0; i < to_angles.size(); i++)
	{
		list.storage(i) = to_angles[i] - from_angles[i];
	}

	PlausibleRoute *pr = new PlausibleRoute(_from, _to, list);
	
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

	return pr;
}

