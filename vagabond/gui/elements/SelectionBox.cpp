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

#include "SelectionBox.h"
#include <iostream>

SelectionBox::SelectionBox() : SimplePolygon()
{
	_renderType = GL_LINES;
	setFragmentShaderFile("assets/shaders/color_only.fsh");
	setVertexShaderFile("assets/shaders/box.vsh");

	addIndex(0);
	addIndex(1);
	addIndex(1);
	addIndex(2);
	addIndex(2);
	addIndex(3);
	addIndex(3);
	addIndex(0);
	
	for (size_t i = 0; i < 4; i++)
	{
		addVertex(glm::vec3(0.f));
	}
	
	setColour(0.f, 0.f, 0.f);
	setAlpha(1.f);
}

/* 0  1 *
 * 3  2 */

void SelectionBox::setBottomRight(float bottom, float right)
{
	_vertices[1].pos.x = right;
	_vertices[2].pos.x = right;
	_vertices[2].pos.y = bottom;
	_vertices[3].pos.y = bottom;
	forceRender(true, false);
}

void SelectionBox::setTopLeft(float top, float left)
{
	_vertices[0].pos.x = left;
	_vertices[3].pos.x = left;
	_vertices[0].pos.y = top;
	_vertices[1].pos.y = top;
	forceRender(true, false);
}

