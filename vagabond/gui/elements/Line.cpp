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

#include "Line.h"

Line::Line() : Renderable()
{
	setName("Line");
	_renderType = GL_LINES;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
}

Snow::Vertex &Line::addPoint(glm::vec3 p)
{
	Vertex &v = addVertex(p);
	
	v.color = glm::vec4(0., 0., 0., 1.);
	if (vertexCount() > 1)
	{
		addIndex(-2);
		addIndex(-1);
	}
	
	return v;
}
