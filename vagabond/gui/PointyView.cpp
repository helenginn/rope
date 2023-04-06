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

#include "PointyView.h"

PointyView::PointyView()
{
	setUsesProjection(true);
	_renderType = GL_POINTS;
	setFragmentShaderFile("assets/shaders/point.fsh");
	setVertexShaderFile("assets/shaders/point.vsh");
	setImage("assets/images/points.png");
}

void PointyView::refresh()
{
	if (_vertices.size())
	{
		makePoints();
	}

	rebufferVertexData();
	
	additionalJobs();
}

void PointyView::customiseTexture(Snow::Vertex &vert)
{
	vert.tex.x = pointTypeCount(); /* number of points */
	vert.tex.y = 0.; /* point index */
	vert.color[3] = 1;
}

void PointyView::setPointType(int idx, int type)
{
	_vertices[idx].tex.y = type; /* point index */
}

void PointyView::addPoint(glm::vec3 pos, int pointType)
{
	Snow::Vertex &vert = addVertex(pos);
	customiseTexture(vert);
	setPointType(_vertices.size() - 1, pointType);

	addIndex(-1);
}

void PointyView::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		/* in the case of multiple responders */
		_vertices[i].extra[0] = i + offset + 1.5;
	}
}

void PointyView::reset()
{
	for (size_t i = 0; i < vertexCount(); i++)
	{
		Snow::Vertex &vert = _vertices[i];
		vert.tex.y = 0.; /* point index */
		vert.color = glm::vec4(0., 0., 0., 1.);
	}
}

void PointyView::extraUniforms()
{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, _size);
	checkErrors("rebinding size");
}

bool PointyView::mouseOver()
{
	interacted(currentVertex(), true, true);
	return (currentVertex() >= 0);
}

void PointyView::unMouseOver()
{
	interacted(-1, true, true);
}
