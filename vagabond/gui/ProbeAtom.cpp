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
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/Decree.h>
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/gui/elements/Menu.h>

using namespace hnet;

void ProbeAtom::fullUpdate()
{
	glm::vec3 c = _probe->colour();
	FloatingText::setText(_probe->display());
	FloatingText::correctBox(_probe->mult(), 0.0);
	FloatingText::setPosition(_probe->position());
	FloatingText::setColour(c.x, c.y, c.z);
	FloatingText::setAlpha(_probe->alpha());
	FloatingText::rebufferVertexData();
	FloatingText::rebufferIndexData();
}

void ProbeAtom::updateProbe()
{
	FloatingText::changeText(_probe->display());
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, AtomProbe *probe)
: FloatingText(probe->display(), probe->mult(), 0.0)
{
	FloatingText::setPosition(probe->position());
	_probe = probe;
	_view = view;

	FloatingText::setFragmentShaderFile("assets/shaders/indexed_box.fsh");
	probe->_obj.set_update([this]() { updateProbe(); });
	fullUpdate();
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, HydrogenProbe *probe)
: FloatingText(probe->display(), 25, 0.0)
{
	_view = view;
	_probe = probe;

	FloatingText::setPosition(probe->position());
	FloatingText::setAlpha(probe->alpha());
	FloatingText::setColour(0, 0, 0);

	FloatingText::setFragmentShaderFile("assets/shaders/indexed_box.fsh");
	probe->_obj.set_update([this]() { updateProbe(); });

	fullUpdate();
}

void ProbeAtom::hoverOverAtom()
{
	if (_probe->is_atom())
	{
		AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);
		::Atom *atom = aProbe->atom();
		_view->setInformation(atom->desc());
	}
}

void ProbeAtom::offerHydrogenMenu()
{
	HydrogenProbe *hProbe = static_cast<HydrogenProbe *>(_probe);

	std::vector<Hydrogen::Values> options = hProbe->_obj.values();

	Menu *m = new Menu(_view, this);
	
	for (const Hydrogen::Values &option : options)
	{
		std::ostringstream ss;
		ss << option;
		m->addOption(ss.str(), "setH_" + ss.str());
	}

	_view->setMenu(m);
}

void ProbeAtom::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		FloatingText::setHighlighted(true);
		_view->setActive((FloatingText *)this);
		hoverOverAtom();
	}

	if (!left && !hover && !_probe->is_atom())
	{
		offerHydrogenMenu();
	}
}

void ProbeAtom::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingText::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		FloatingText::_vertices[i].extra[3] = offset + 1.5;
	}
}

void ProbeAtom::declareHydrogen(Hydrogen::Values value)
{
	std::string name = "Declare hydrogen present";
	Decree *d = _view->network().newDecree(name);

	HydrogenProbe *hProbe = static_cast<HydrogenProbe *>(_probe);
	hProbe->_obj.assign_value(value, d, d);
}

void ProbeAtom::buttonPressed(std::string tag, Button *button)
{
	if (tag == "setH_Present")
	{
		declareHydrogen(Hydrogen::Present);
	}
	else if (tag == "setH_Absent")
	{
		declareHydrogen(Hydrogen::Absent);
	}

}
