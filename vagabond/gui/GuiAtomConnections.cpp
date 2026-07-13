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

#include "GuiAtomConnections.h"

#include <vagabond/core/Atom.h>
#include <vagabond/core/matrix_functions.h>
#include <algorithm>
#include <cmath>

GuiAtomConnections::GuiAtomConnections(GuiAtom *parent) : GuiRepresentation(parent)
{
	setName("GuiAtomConnections");
	_renderType = GL_TRIANGLES;
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	setImage("assets/images/bond.png");
}

glm::vec3 GuiAtomConnections::positionFor(Atom *atom) const
{
	glm::vec3 pos = atom->derivedPosition();
	if (!is_glm_vec_sane(pos))
	{
		pos = atom->initialPosition();
	}

	return pos;
}

void GuiAtomConnections::addConnection(Atom *left, Atom *right)
{
	if (!left || !right || left == right)
	{
		return;
	}

	AtomMarker marker{left, right, vertexCount()};
	addArrow(positionFor(left), positionFor(right), 0.85f);

	_atom2Markers[left].push_back(_markers.size());
	_atom2Markers[right].push_back(_markers.size());
	_markers.push_back(marker);
}

void GuiAtomConnections::setConnections(
const std::vector<AtomConnection> &connections)
{
	clearVertices();
	_atom2Markers.clear();
	_markers.clear();
	_hidden.clear();

	_vertices.reserve(connections.size() * 12);
	_indices.reserve(connections.size() * 18);

	for (const AtomConnection &connection : connections)
	{
		addConnection(connection.first, connection.second);
	}

	forceRender(true, true);
}

void GuiAtomConnections::updateMarker(AtomMarker &marker)
{
	if (marker.idx + 1 >= _vertices.size())
	{
		return;
	}

	bool hidden = (_hidden.count(marker.left) || _hidden.count(marker.right));
	float alpha = hidden ? 0.12f : 0.85f;
	setArrow(marker.idx, positionFor(marker.left), positionFor(marker.right),
	         alpha);
}

void GuiAtomConnections::addArrow(glm::vec3 start, glm::vec3 end, float alpha)
{
	size_t idx = vertexCount();
	for (size_t i = 0; i < 3; i++)
	{
		addSegment(start, end, glm::vec4(0.f), 1.f);
	}

	setArrow(idx, start, end, alpha);
}

void GuiAtomConnections::addSegment(glm::vec3 start, glm::vec3 end,
                                    const glm::vec4 &colour, float half_width)
{
	glm::vec3 dir = end - start;
	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = -half_width;
		v.tex[1] = 0.f;
		v.color = colour;
	}

	{
		Snow::Vertex &v = addVertex(end);
		v.normal = dir;
		v.tex[0] = -half_width;
		v.tex[1] = 1.f;
		v.color = colour;
	}

	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = half_width;
		v.tex[1] = 0.f;
		v.color = colour;
	}

	{
		Snow::Vertex &v = addVertex(end);
		v.normal = dir;
		v.tex[0] = half_width;
		v.tex[1] = 1.f;
		v.color = colour;
	}

	addIndices(-4, -3, -2);
	addIndices(-2, -3, -1);
}

void GuiAtomConnections::setArrow(size_t idx, glm::vec3 start, glm::vec3 end,
                                  float alpha)
{
	glm::vec3 dir = end - start;
	float len = glm::length(dir);
	if (len < 1e-4f)
	{
		return;
	}

	glm::vec3 unit = dir / len;
	float head_len = std::min(len * 0.35f, 1.2f);
	float head_width = head_len * 0.35f;

	glm::vec3 up = glm::vec3(0.f, 0.f, 1.f);
	if (fabs(glm::dot(unit, up)) > 0.85f)
	{
		up = glm::vec3(0.f, 1.f, 0.f);
	}

	glm::vec3 side = glm::normalize(glm::cross(unit, up));
	glm::vec3 tip = end;
	glm::vec3 shaft_end = end - unit * (head_len * 0.65f);
	glm::vec3 head_base = end - unit * head_len;
	glm::vec3 left = head_base + side * head_width;
	glm::vec3 right = head_base - side * head_width;
	glm::vec4 colour(0.15f, 0.7f, 1.0f, alpha);

	setSegment(idx + 0, start, shaft_end, colour, 1.05f);
	setSegment(idx + 4, left, tip, colour, 1.35f);
	setSegment(idx + 8, right, tip, colour, 1.35f);
}

void GuiAtomConnections::setSegment(size_t idx, glm::vec3 start, glm::vec3 end,
                                    const glm::vec4 &colour, float half_width)
{
	if (idx + 3 >= _vertices.size())
	{
		return;
	}

	glm::vec3 dir = end - start;
	glm::vec2 tex_x{-half_width, half_width};

	_vertices[idx + 0].pos = start;
	_vertices[idx + 1].pos = end;
	_vertices[idx + 2].pos = start;
	_vertices[idx + 3].pos = end;

	_vertices[idx + 0].tex = glm::vec2(tex_x.x, 0.f);
	_vertices[idx + 1].tex = glm::vec2(tex_x.x, 1.f);
	_vertices[idx + 2].tex = glm::vec2(tex_x.y, 0.f);
	_vertices[idx + 3].tex = glm::vec2(tex_x.y, 1.f);

	for (size_t i = 0; i < 4; i++)
	{
		_vertices[idx + i].normal = dir;
		_vertices[idx + i].color = colour;
	}
}

void GuiAtomConnections::updateSinglePosition(Atom *a, glm::vec3 &p)
{
	if (_atom2Markers.count(a) == 0)
	{
		return;
	}

	for (size_t idx : _atom2Markers[a])
	{
		updateMarker(_markers[idx]);
	}
}

void GuiAtomConnections::updateMultiPositions(Atom *a, WithPos &wp)
{
	updateSinglePosition(a, wp.ave);
}

void GuiAtomConnections::setHidden(Atom *a, const bool &hidden)
{
	if (hidden)
	{
		_hidden.insert(a);
	}
	else
	{
		_hidden.erase(a);
	}

	if (_atom2Markers.count(a) == 0)
	{
		return;
	}

	for (size_t idx : _atom2Markers[a])
	{
		updateMarker(_markers[idx]);
	}
}
