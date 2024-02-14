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

#include <iostream>
#include "FloatingText.h"
#include "Library.h"

FloatingText::FloatingText(std::string text, float mult, float yOff) : Text(text)
{
	setVertexShaderFile("assets/shaders/floating_box.vsh");
	correctBox(mult, yOff);
	setUsesProjection(true);
}

void FloatingText::correctBox(float mult, float y_offset)
{
	glm::vec3 centre = centroid();

	for (Snow::Vertex &v : _vertices)
	{
		v.extra = glm::vec4(v.pos - centre, 0);
		v.extra *= mult;
		
		v.extra.y += y_offset;
		v.pos = centre;
	}
}

void FloatingText::changeText(const std::string &text)
{
	float old_w = _w;
	float old_h = _h;

	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	GLuint tex = Library::getLibrary()->loadText(text, &_w, &_h, _type);
	_texid = tex;
	
	glm::mat3x3 mat = glm::mat3(1.);
	mat[0][0] = (float)_w / old_w;
	mat[1][1] = (float)_h / old_h;

	for (Vertex &v : _vertices)
	{
		float last = v.extra[3];
		v.extra = glm::vec4(mat * glm::vec3(v.extra), last);
	}

	for (Vertex &v : _unselectedVertices)
	{
		float last = v.extra[3];
		v.extra = glm::vec4(mat * glm::vec3(v.extra), last);
	}

	FloatingText::forceRender(true, true);
}
