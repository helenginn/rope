// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "ProbeBondBatch.h"
#include <vagabond/gui/elements/Window.h>
#include <iostream>

namespace
{

std::vector<Snow::Vertex> unit_bond_vertices()
{
	Snow::Vertex v = {};
	std::vector<Snow::Vertex> vs(4, v);

	vs[0].tex[0] = -0.5; vs[0].tex[1] = 0;
	vs[1].tex[0] = -0.5; vs[1].tex[1] = 1;
	vs[2].tex[0] = 0.5;  vs[2].tex[1] = 0;
	vs[3].tex[0] = 0.5;  vs[3].tex[1] = 1;

	// identity atlas sub-rect (whole image) until setSlotImage() picks a
	// real one - see axes_atlas.fsh.
	for (Snow::Vertex &vert : vs)
	{
		vert.extra[0] = 0.f;
		vert.extra[1] = 1.f;
		vert.extra[2] = 1.f;
	}

	return vs;
}

std::vector<GLuint> unit_bond_indices()
{
	return {0, 1, 2, 1, 2, 3};
}

}

ProbeBondBatch::ProbeBondBatch()
: IndexedBatch(unit_bond_vertices(), unit_bond_indices(), 3)
{
	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes_atlas.fsh");
	setUsesProjection(true);
	setName("Probe bond batch");

	// the full closed set of names BondProbe::display()/CovalentProbe::
	// display() can return - see this class's header comment.
	static const std::vector<std::string> names = {
		"weak_bond", "strong_bond", "lone_pair", "transparency",
		"present_bond", "unassigned_bond", "double_bond", "single_bond",
		"single_or_double_bond",
	};

	std::vector<std::string> paths;
	paths.reserve(names.size());
	for (const std::string &name : names)
	{
		paths.push_back("assets/images/" + name + ".png");
	}

	_texid = Library::getLibrary()->buildAtlas("probe_bond_atlas", paths,
	                                           _rects);

	// buildAtlas() keys _rects by the path it was given, but
	// setSlotImage() is called with the bare display() name - reindex to
	// that key instead of changing what buildAtlas() returns generally.
	std::map<std::string, AtlasRect> byName;
	for (const std::string &name : names)
	{
		std::string path = "assets/images/" + name + ".png";
		if (_rects.count(path))
		{
			byName[name] = _rects[path];
		}
	}
	_rects = byName;
}

size_t ProbeBondBatch::appendBond(BatchHandle *handle)
{
	return appendSlot(handle);
}

void ProbeBondBatch::fixVertices(size_t slot, const glm::vec3 &start,
                                 const glm::vec3 &dir)
{
	size_t i = vertexOffset(slot);

	_vertices[i + 0].pos = start;       _vertices[i + 0].normal = dir;
	_vertices[i + 1].pos = start + dir; _vertices[i + 1].normal = dir;
	_vertices[i + 2].pos = start;       _vertices[i + 2].normal = dir;
	_vertices[i + 3].pos = start + dir; _vertices[i + 3].normal = dir;
}

void ProbeBondBatch::setSlotImage(size_t slot, const std::string &imageName)
{
	if (!_rects.count(imageName))
	{
		std::cout << "ProbeBondBatch: no atlas entry for '" << imageName
		<< "'" << std::endl;
		return;
	}

	const AtlasRect &r = _rects.at(imageName);
	size_t i = vertexOffset(slot);

	for (size_t j = i; j < i + verticesPerSlot(); j++)
	{
		_vertices[j].extra[0] = r.u0;
		_vertices[j].extra[1] = r.u1 - r.u0;
		_vertices[j].extra[2] = r.v1 - r.v0;
	}

	// unlike Image::changeImage() (a texture id swap, re-read fresh every
	// draw call), this writes into the buffered vertex array, so the GPU
	// side needs telling to re-upload it.
	forceRender(true, false);
}

void ProbeBondBatch::setSlotColour(size_t slot, double r, double g, double b)
{
	size_t i = vertexOffset(slot);

	for (size_t j = i; j < i + verticesPerSlot(); j++)
	{
		_vertices[j].color[0] = r;
		_vertices[j].color[1] = g;
		_vertices[j].color[2] = b;
	}

	forceRender(true, false);
}

void ProbeBondBatch::setSlotAlpha(size_t slot, double alpha)
{
	size_t i = vertexOffset(slot);

	for (size_t j = i; j < i + verticesPerSlot(); j++)
	{
		_vertices[j].color[3] = alpha;
	}

	forceRender(true, false);
}

void ProbeBondBatch::extraUniforms()
{
	GLint u = glGetUniformLocation(_program, "aspect");
	glUniform1f(u, Window::aspect());
}
