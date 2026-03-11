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

#include "TopologyCircle.h"
#include "ModelTopologyView.h"

TopologyCircle::TopologyCircle(ModelTopologyView *view) 
: FloatingImage("assets/images/circle.png", 10), 
_view(view)
{
#ifndef __EMSCRIPTEN__
	std::string shader = "assets/shaders/indexed_box.fsh";
#else
	std::string shader = "assets/shaders/box.fsh";
#endif
	FloatingImage::setFragmentShaderFile(shader);

}

void TopologyCircle::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < FloatingImage::vertexCount(); i++)
	{
		/* in the case of multiple responders */
		FloatingImage::_vertices[i].extra[3] = offset + 1.5;
	}
}

void TopologyCircle::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		_view->unhighlight();
		FloatingImage::setHighlighted(true);
//		_view->setActive((FloatingText *)this);
//		hoverOverAtom();
	}

	if (left && !hover)
	{
		_view->clicked(this);
//		offerHydrogenMenu();
	}
}
