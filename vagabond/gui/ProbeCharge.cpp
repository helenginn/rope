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

#include <vagabond/core/protonic/CountProbe.h>
#include "ProbeCharge.h"

ProbeCharge::ProbeCharge(ProtonNetworkView *view, CountProbe *probe)
: FloatingImage("assets/images/" + probe->display() + ".png", 1, probe->_offset) 
{
	_view = view;
	_probe = probe;

	_probe->_obj.set_update([this](bool thorough)
	                       { thorough ? fullUpdate() : quickUpdate(); });
	_probe->_exist.add_update([this](bool thorough)
	                          { thorough ? fullUpdate() : quickUpdate(); });
	fullUpdate();
}

void ProbeCharge::quickUpdate()
{
	FloatingImage::addMainThreadJob
	([this]()
	{
		FloatingImage::changeImage("assets/images/" + _probe->display() + ".png");
		FloatingImage::setAlpha(_probe->alpha());
		FloatingImage::forceRender(true, true);
	});
}

void ProbeCharge::fullUpdate()
{
	FloatingImage::setPosition(_probe->position());
	FloatingImage::changeImage("assets/images/" + _probe->display() + ".png");
	FloatingImage::setAlpha(_probe->alpha());
	FloatingImage::forceRender(true, true);
}


void ProbeCharge::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingImage::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		FloatingImage::_vertices[i].extra[3] = offset + 1.5;
	}

}
