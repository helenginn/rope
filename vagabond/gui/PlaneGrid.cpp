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

#include "PlaneGrid.h"

PlaneGrid::PlaneGrid(Plane *p) : Renderable()
{
	setName("Plane view");
	_renderType = GL_LINES;
	_plane = p;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");

}

void PlaneGrid::populate()
{
	clearVertices();

	int nx = _plane->num(0);
	int ny = _plane->num(1);
	
	bool connect_left = false;
	bool connect_top = false;

	for (int y = 0; y < ny; y++)
	{
		connect_left = false;

		for (int x = 0; x < nx; x++)
		{
			glm::vec3 v = _plane->generateVertex(x, y);
			addVertex(v);
			
			if (connect_left)
			{
				addIndex(-2);
				addIndex(-1);
			}
			
			if (connect_top)
			{
				addIndex(-ny - 1);
				addIndex(-1);
			}
			
			connect_left = true;
		}
		
		connect_top = true;
	}
	
	rebufferVertexData();
	rebufferIndexData();
	setColour(0., 0., 0.);
	setAlpha(1.);
	
}
