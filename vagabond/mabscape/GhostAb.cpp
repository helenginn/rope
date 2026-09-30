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

#include "GhostAb.h"
#include <vagabond/gui/elements/Icosahedron.h>
#include <SDL2/SDL.h>
#include <cmath>

GhostAb::GhostAb()
{
	// the same shape and size as the antibodies themselves
	Icosahedron ico;
	ico.triangulate();
	_vertices = ico.vertices();
	_indices = ico.indices();

	glm::vec3 centre = centroid();
	for (Vertex &v : _vertices)
	{
		_offsets.push_back(v.pos - centre);
		v.color = glm::vec4(0.f, 0.f, 0.f, 0.35f);
	}

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/lighting.fsh");
	setDisabled(true);
}

void GhostAb::show(const glm::vec3 &where)
{
	_centre = where;
	setDisabled(false);
}

void GhostAb::hide()
{
	setDisabled(true);
}

void GhostAb::render(GLView *gl)
{
	if (isDisabled())
	{
		Renderable::render(gl);
		return;
	}

	// the radius follows a sine wave in time, about 1.2 seconds a cycle
	float t = SDL_GetTicks() / 1000.f;
	float scale = 1.f + 0.3f * sin(t * 5.f);

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].pos = _centre + _offsets[i] * scale;
	}

	forceRender(true, false);

	// the near and far sides of the sphere would pile up as they are both
	// translucent: find the depth of the nearest, then blend only that layer
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	Renderable::render(gl);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	Renderable::render(gl);

	// the antibodies drawn afterwards should not be hidden by it
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
}

GhostLine::GhostLine()
{
	_renderType = GL_LINES;
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
	setDisabled(true);
}

void GhostLine::show(const glm::vec3 &from, const glm::vec3 &to)
{
	_vertices.clear();
	_indices.clear();

	for (const glm::vec3 &end : {from, to})
	{
		Vertex &v = addVertex(end);
		v.color = glm::vec4(0.f, 0.f, 0.f, 0.8f);
	}

	addIndices(0, 1);
	setDisabled(false);
	forceRender(true, true);
}

void GhostLine::hide()
{
	setDisabled(true);
}
