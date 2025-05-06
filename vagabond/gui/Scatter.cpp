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

#include "Scatter.h"
#include "VagWindow.h"

Scatter::Scatter()
{
	_renderType = GL_POINTS;
	setFragmentShaderFile("assets/shaders/point.fsh");
	setVertexShaderFile("assets/shaders/point.vsh");
	setImage("assets/images/points.png");
	_size *= Window::ratio() / 2;
}

void Scatter::addPoint(glm::vec3 p, glm::vec3 colour, int pointType)
{
	if (!usesProjection())
	{
		p.x = p.x * 2 - 1;
		p.y = 1 - p.y * 2;
	}

	Snow::Vertex &vert = addVertex(p);

	vert.tex.x = 8; /* number of points */
	vert.tex.y = 0.; /* point index */
	vert.color = glm::vec4(colour, 1.f);
	int idx = _vertices.size() - 1;
	_vertices[idx].tex.y = pointType; /* point index */
	addIndex(-1);
}

void Scatter::extraUniforms()
{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, _size);
}

