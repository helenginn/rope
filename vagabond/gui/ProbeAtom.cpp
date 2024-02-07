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
#include <vagabond/core/protonic/Probe.h>

ProbeAtom::ProbeAtom(ProtonNetworkView *view, AtomProbe *probe)
: FloatingText(probe->display(), probe->mult(), 0.0)
{
	FloatingText::setPosition(probe->position());
	glm::vec3 c = probe->colour();
	FloatingText::setColour(c.x, c.y, c.z);
	FloatingText::setAlpha(probe->alpha());
	_probe = probe;
	_view = view;

	FloatingText::setFragmentShaderFile("assets/shaders/indexed_box.fsh");
}

ProbeAtom::ProbeAtom(ProtonNetworkView *view, HydrogenProbe *probe)
: FloatingText(probe->display(), 25, 0.0)
{
	_view = view;

	FloatingText::setPosition(probe->position());
	FloatingText::setAlpha(probe->alpha());
	FloatingText::setColour(0, 0, 0);

	FloatingText::setFragmentShaderFile("assets/shaders/indexed_box.fsh");
}

void ProbeAtom::hoverOverAtom()
{
//	AtomProbe *aProbe = static_cast<AtomProbe *>(_probe);
//	::Atom *atom = aProbe->atom();
//	_view->setInformation(atom->desc());
}

void ProbeAtom::offerAtomMenu()
{

}

void ProbeAtom::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		FloatingText::setHighlighted(true);
		_view->setActive((FloatingText *)this);
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
