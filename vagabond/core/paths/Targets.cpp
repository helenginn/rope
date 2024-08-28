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

#include "Targets.h"
#include "Route.h"
#include "Instance.h"
#include "AtomGroup.h"

Targets::Targets(Route *const &route, const std::vector<InstancePair> &pairs) 
{
	_pairs = pairs;
	_route = route;
}

void Targets::setEndPoint(InstancePair &pair)
{
	_route->submitToShow(0.);
	_route->retrieve();

	std::map<Atom *, glm::vec3> atomStart;
	AtomGroup *start = pair.start->currentAtoms();
	AtomGroup *end = pair.end->currentAtoms();
	end->recalculate();

	for (Atom *atom : start->atomVector())
	{
		Atom *corresponding = pair.end->equivalentForAtom(pair.start, atom);
		if (corresponding)
		{
			glm::vec3 d = corresponding->derivedPosition();
			atom->setOtherPosition("end", d);
		}
	}
}

void Targets::setTargets(InstancePair &pair)
{
	std::map<Atom *, glm::vec3> atomStart;
	AtomGroup *grp = pair.start->currentAtoms();

	_route->submitToShow(0.);
	_route->retrieve();

	for (Atom *atom : grp->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		atomStart[atom] = d;
	}

	_route->submitToShow(1.);
	_route->retrieve();

	for (Atom *atom : grp->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		glm::vec3 s = atomStart[atom];
		glm::vec3 diff = (d - s);
		atom->setOtherPosition("moving", diff);
		atom->setOtherPosition("target", s);
	}
}

void Targets::operator()()
{
	for (InstancePair &pair : _pairs)
	{
		pair.start->load();
		pair.end->load();

		setTargets(pair);
		setEndPoint(pair);

		pair.start->unload();
		pair.end->unload();
	}
}
