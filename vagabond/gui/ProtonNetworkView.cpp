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

#include "ProbeBond.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/gui/elements/FloatingText.h>

ProtonNetworkView::ProtonNetworkView(Scene *scene, Network &network) 
: Scene(scene), Mouse3D(scene), _network(network)
{
	_translation.z -= 50;
	_farSlab = 40;
	_slabbing = true;
}

void ProtonNetworkView::findAtomProbes()
{
	for (AtomProbe *const &probe : _network.atomProbes())
	{
		FloatingText *text = new FloatingText(probe->display(), 
		                                      probe->mult(), -0.0);
		text->setPosition(probe->position());
		glm::vec3 c = probe->colour();
		text->setColour(c.x, c.y, c.z);
		addObject(text);
	}

	for (HydrogenProbe *const &probe : _network.hydrogenProbes())
	{
		FloatingText *text = new FloatingText(probe->display(), 25, -0.0);
		text->setPosition(probe->position());
		addObject(text);
	}

	for (BondProbe *const &probe : _network.bondProbes())
	{
		ProbeBond *bond = new ProbeBond(probe->display(), probe->position(),
		                                probe->end());
		addObject(bond);
	}

	shiftToCentre(_network.centre(), 50);
}

void ProtonNetworkView::setup()
{
	addTitle("Proton network");

	findAtomProbes();
}
