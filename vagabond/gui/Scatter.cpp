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

#include <vagabond/gui/elements/FloatingText.h>
#include "Graph.h"
#include "Scatter.h"
#include "VagWindow.h"

Scatter::Scatter(Graph *graph, int index)
{
	_graph = graph;
	_renderType = GL_POINTS;
	setFragmentShaderFile("assets/shaders/point.fsh");
	setVertexShaderFile("assets/shaders/point.vsh");
	setImage("assets/images/points.png");
	_size *= Window::ratio() / 2;
}

Scatter::~Scatter()
{
	if (_view)
	{
		_view->removeResponder(this);
	}
}

void Scatter::addPoint(glm::vec3 p, glm::vec3 colour, int pointType,
                       float alpha)
{
	if (!usesProjection())
	{
		p.x = p.x * 2 - 1;
		p.y = 1 - p.y * 2;
	}

	Snow::Vertex &vert = addVertex(p);

	vert.tex.x = 8; /* number of points */
	vert.tex.y = 0.; /* point index */
	vert.color = glm::vec4(colour, alpha);
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

void Scatter::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		/* in the case of multiple responders */
		_vertices[i].extra[0] = i + offset + 1.5;
	}

	forceRender(true, false);
}

void Scatter::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		_graph->noteLabelShown();
		deleteTemps();
		std::string str = _graph->data_label(_index, idx);
		Text *t = new Text(str);
		glm::vec3 pos = _vertices[idx].pos;
		pos.y += 0.04;
		t->setPosition(pos);
		addTempObject(t);
	}

}
