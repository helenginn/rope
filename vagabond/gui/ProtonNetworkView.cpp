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

#include "ProbeAtom.h"
#include "ProbeBond.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/gui/elements/FloatingText.h>
#include <vagabond/gui/elements/Menu.h>

ProtonNetworkView::ProtonNetworkView(Scene *scene, Network &network) 
: Scene(scene), Mouse3D(scene), IndexResponseView(scene), _network(network)
{
	_translation.z -= 50;
	_farSlab = 40;
	_slabbing = true;
}

void ProtonNetworkView::findAtomProbes()
{
	for (AtomProbe *const &probe : _network.atomProbes())
	{
		ProbeAtom *text = new ProbeAtom(this, probe);
		addObject((FloatingText *)text);
		_textProbes[probe] = text;
		probe->setResponder(this);
		addIndexResponder(text);
	}

	for (HydrogenProbe *const &probe : _network.hydrogenProbes())
	{
		ProbeAtom *text = new ProbeAtom(this, probe);
		addObject((FloatingText *)text);
		_textProbes[probe] = text;
		probe->setResponder(this);
		addIndexResponder(text);
	}

	for (BondProbe *const &probe : _network.bondProbes())
	{
		ProbeBond *bond = new ProbeBond(this, probe);
		addObject((Image *)bond);
		_bondProbes[probe] = bond;
		probe->setResponder(this);
		addIndexResponder(bond);
	}

	shiftToCentre(_network.centre(), 50);
	IndexResponseView::setup();
}

void ProtonNetworkView::interactedWithNothing(bool left, bool hover)
{
	if (_active)
	{
		_active->setHighlighted(false);
		_active = nullptr;
	}
	setInformation("");
}

void ProtonNetworkView::sendObject(std::string tag, void *object)
{
	std::cout << tag << std::endl;
	Probe *p = static_cast<Probe *>(object);
	if (_textProbes.count(p))
	{
		_textProbes[p]->FloatingText::setAlpha(p->alpha());
	}

	if (_bondProbes.count(p))
	{
		_bondProbes[p]->updateProbe();
	}
}

void ProtonNetworkView::setup()
{
	addTitle("Proton network");

	findAtomProbes();
}

void ProtonNetworkView::setMenu(Menu *menu)
{
	float x; float y;
	getFractionalPos(x, y);
	menu->setup(x, y);

	setModal(menu);
}
