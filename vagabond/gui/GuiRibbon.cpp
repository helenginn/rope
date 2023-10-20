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

#include "GuiRibbon.h"
#include "GuiAtom.h"
#include <vagabond/gui/elements/Library.h>
#include <vagabond/utils/maths.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/RopeTypes.h>
#include <vagabond/core/ResidueId.h>
#include <vagabond/core/RopeTypes.h>

#define INDICES_PER_BEZIER_DIVISION (12)
#define DIVISIONS_IN_CIRCLE (16)

GuiRibbon::GuiRibbon(GuiAtom *parent) : GuiRepresentation(parent)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
//	setFragmentShaderFile("assets/shaders/lighting.fsh");
	setFragmentShaderFile("assets/shaders/pencil_shading.fsh");

	std::string filename = "assets/images/pencil_shading.png";
	setImage(filename, true);

	insertAtom(nullptr);
}

GuiRibbon::~GuiRibbon()
{

}

void GuiRibbon::extraUniforms()
{
	const char *uniform_name = "dot_threshold";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	float dot = 0.3;
	glUniform1f(u, dot);
}


std::vector<Snow::Vertex> GuiRibbon::addCircle(glm::vec3 centre, 
                                               const std::vector<glm::vec3> &circle)
{
	std::vector<Vertex> vertices;
	for (size_t i = 0; i < circle.size(); i++)
	{
		glm::vec3 pos = centre + circle[i];
		Snow::Vertex v = new_vertex(pos);
		v.normal = circle[i];
		vertices.push_back(v);
	}
	return vertices;
}

void GuiRibbon::addCylinderIndices(size_t num)
{
	GuiRepresentation::addCylinderIndices(_vertices, _indices, num);
}

void GuiRibbon::segmentToWatch(std::vector<glm::vec3> &segment,
                               GuiRibbon::Watch::Entry &source,
                               GuiRibbon::Watch &watch)
{
	if (source.next_idx < 0) // next chunk doesn't exist yet, append to array
	{
		int bez_idx = _bezier.size(); // first index of bezier for this atom
		for (size_t j = 0; j < segment.size(); j++)
		{
			Watch::Entry entry(source);
			entry.pos = segment[j];
			watch.addWatchedAtom(entry);
		}
		source.next_idx = bez_idx; // prepare for recall
	}
	else // modify existing array, positions only
	{
		for (size_t j = 0; j < segment.size(); j++)
		{
			int write_idx = j + source.next_idx;
			Watch::Entry &entry = watch[write_idx];
			entry.pos = segment[j];
		}
	}
}

struct Circle
{
	Circle(glm::vec3 &offAxis, const float &radius, 
	       const int &divisions, const glm::vec3 &axis)
	{
		float angle = (2 * M_PI) / (double)divisions;
		glm::mat4x4 rot = glm::rotate(glm::mat4(1.f), angle, axis);
		glm::vec3 cross = glm::normalize(glm::cross(axis, offAxis));
		offAxis = glm::normalize(glm::cross(cross, axis));
		cross *= radius;

		for (size_t i = 0; i < divisions; i++)
		{
			_ret.push_back(cross);
			cross = glm::vec3(rot * glm::vec4(cross, 1.));
		}
	};
	
	std::vector<glm::vec3> operator()()
	{
		return _ret;
	}

	std::vector<glm::vec3> _ret;
};

struct HasAtom
{
	HasAtom(GuiRibbon::Watch &atoms) : _atoms(atoms)
	{
	
	}
	
	bool operator()(int idx) const
	{
		if (idx < 0 || idx >= _atoms.size())
		{
			return false;
		}

		return (_atoms[idx].atom != nullptr);

	}
	
	GuiRibbon::Watch &_atoms;
};

std::vector<Vertex> GuiRibbon::verticesForCylinder(int i)
{
	glm::vec3 xAxis = {1, 0, 0};

	const int divisions = DIVISIONS_IN_CIRCLE;
	Circle pucker = Circle(xAxis, 0, divisions, glm::vec3{});

	auto has_atom = HasAtom(_bezier);
	if (!has_atom(i))
	{
		return std::vector<Snow::Vertex>();
	}

	glm::vec3 centre = _bezier[i].pos;
	std::vector<Snow::Vertex> vertices;

	// if we back onto a nullptr, end of chain = pucker.
	if (!has_atom(i - 1) || !has_atom(i + 1))
	{
		vertices = addCircle(centre, pucker());
	}
	else
	{
		glm::vec3 v1 = _bezier[i - 1].pos;
		glm::vec3 prev = _bezier[i - 1].prev_axis;
		glm::vec3 v2 = _bezier[i + 1].pos;

		/* get the axis of motion */
		glm::vec3 axis = glm::normalize(v2 - v1);

		/* generate the appropriate circle params */
		Circle circle = Circle(prev, 0.3 * _bezier[i].radius, 
		                       divisions, axis);
		
		_bezier[i].prev_axis = prev;
		vertices = addCircle(centre, circle());
	}
	
	colourCylinderVertices(vertices, _bezier[i].atom);

	return vertices;
}

// index i refers to the _bezier indexing
void GuiRibbon::prepareCylinder(int i)
{
	if (i < 0 || i > _bezier.size())
	{
		return;
	}

	std::vector<Vertex> vertices = verticesForCylinder(i);
	
	if (vertices.size() == 0)
	{
		return;
	}

	int cyl_idx = _bezier[i].next_idx;

	if (cyl_idx < 0 && _bezier[i].atom)
	{
		cyl_idx = vertexCount();
		_vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
		_bezier.setNextIndex(i, cyl_idx);

		addCylinderIndices(DIVISIONS_IN_CIRCLE);
	}
	else
	{
		int k = 0;
		for (size_t j = cyl_idx; j < vertices.size() + cyl_idx; j++)
		{
			_vertices[j] = vertices[k];
			k++;
		}
	}
}

void GuiRibbon::prepareCylinder()
{
	for (int i = 1; i < (int)_bezier.size() - 1; i++)
	{
		prepareCylinder(i);
	}
}

bool GuiRibbon::correct_indices(int *is, GuiRibbon::Watch &atoms)
{
	auto has_atom = HasAtom(_atoms);

	if (!has_atom(is[1]) || !has_atom(is[2]))
	{
		return false;
	}

	if (!has_atom(is[0]))
	{
		is[0] = is[1];
	}

	if (!has_atom(is[3]))
	{
		is[3] = is[2];
	}

	return true;
}

std::vector<glm::vec3> GuiRibbon::bezierSegment(glm::vec3 p1, glm::vec3 p2,
                                                glm::vec3 p3, glm::vec3 p4)
{
	std::vector<glm::vec3> points;
	float t = 0;

	for (int i = 0; i < POINTS_PER_BEZIER; i++)
	{
		glm::vec3 p = bezier(p1, p2, p3, p4, t);
		t += 1 / (float)(POINTS_PER_BEZIER);
		points.push_back(p);
	}

	return points;

}

void GuiRibbon::prepareBezier(int i)
{
	// get all four atoms
	int is[] = {i - 1, i + 0, i + 1, i + 2};

	std::vector<glm::vec3> segment;

	if (!correct_indices(is, _atoms))
	{
		/* add a null watch so we know when to pucker the ribbon */
		segment = {glm::vec3{NAN}};
	}
	else
	{
		glm::vec3 ps[4];
		for (size_t i = 0; i < 4; i++)
		{
			ps[i] = _atoms[is[i]].pos;
		}

		control_points(&ps[0], ps[1], ps[2], &ps[3]);
		segment = bezierSegment(ps[0], ps[1], ps[2], ps[3]);
	}


	segmentToWatch(segment, _atoms[i], _bezier);
	
}

void GuiRibbon::prepareBezier()
{
	_bezier.addWatchedAtom(Watch::Entry{});

	// skip the first and the last atoms
	for (int i = 1; i < (int)_atoms.size() - 2; i++)
	{
		prepareBezier(i);
	}

	_bezier.addWatchedAtom(Watch::Entry{});
}

bool GuiRibbon::previousPositionValid()
{
	bool lastOK = (_vertices.size() > 0 &&
			_vertices.back().pos[0] == _vertices.back().pos[0]);

	return lastOK;
}

void GuiRibbon::insertAtom(Atom *a)
{
	Watch::Entry entry{};
	
	bool has = _atoms.has(a);
	
	if (a)
	{
		entry.pos = a->initialPosition();
		entry.atom = a;
		rope::MonomerType type = _group->monomerType(a->residueId());
		entry.radius = (type == rope::IsAminoAcid ? 1 : 3);
	}
	
	_atoms.addWatchedAtom(entry);
}

bool GuiRibbon::acceptableAtom(Atom *a)
{
	if (a->atomName() == "CA")
	{
		return true;
	}

	if (a->atomName() == "P")
	{
		return true;
	}

	return false;
}

void GuiRibbon::watchAtom(Atom *a)
{
	if (!a || !a->isReporterAtom())
	{
		return;
	}

	rope::MonomerType type = _group->monomerType(a->residueId());
	int max_separation = (type == rope::IsAminoAcid ? 4 : 6);

	Atom *prev = _atoms.lastAtom();
	int separation = -1;

	if (prev)
	{
		separation = a->bondsBetween(prev, 6);
	}

	if (!prev || (separation <= max_separation && separation >= 0))
	{
		insertAtom(a);
	}
	else
	{
		insertAtom(nullptr);
		insertAtom(a);
	}

}
void GuiRibbon::convert()
{
	insertAtom(nullptr);
	insertAtom(nullptr);
	prepareBezier();
	prepareCylinder();
	forceRender(true, true);
	finishUpdate();
}

void GuiRibbon::transplantCylinder(std::vector<Snow::Vertex> &cylinder, int start)
{
	if (_vertices.size() <= start + cylinder.size())
	{
		_vertices.resize(start + cylinder.size());
	}

	std::copy(cylinder.begin(), cylinder.end(), _vertices.begin() + start);
}

size_t GuiRibbon::indicesPerAtom()
{
	return POINTS_PER_BEZIER * DIVISIONS_IN_CIRCLE * INDICES_PER_BEZIER_DIVISION;
}

size_t GuiRibbon::verticesPerAtom()
{
	return POINTS_PER_BEZIER * DIVISIONS_IN_CIRCLE;
}

void GuiRibbon::prepareAtomSpace(AtomGroup *ag)
{
	int count = 0;
	_group=ag;
	for (size_t i = 0; i < ag->size(); i++)
	{
		// HARDCODE
		if (((*ag)[i])->isReporterAtom())
		{
			count++;
		}
	}

	_vertices.reserve(count * verticesPerAtom());
	_indices.reserve(count * indicesPerAtom());
}

void GuiRibbon::colourCylinderVertices(std::vector<Vertex> &vertices, Atom *a)
{
	float prop = a->addedColour();
	glm::vec4 c = _scheme->colour(prop);

	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].color = c;
		vertices[i].extra[3] = prop * 3;
	}
}

void GuiRibbon::updateSinglePosition(Atom *a, glm::vec3 &p)
{
	if (!_atoms.has(a))
	{
		return;
	}

	int fix = _atoms.index(a);
	_atoms[a].pos = p;
	_atoms[a].changed = true;

	for (int i = fix - 2; i < fix + 1; i++)
	{
		if (i < 0 || i >= (int)_atoms.size() - 1)
		{
			continue;
		}

		prepareBezier(i);

		int start = _atoms[i].next_idx;

		if (start < 0) { continue; }

		int end = _atoms.stopIndex(i);

		for (int j = start; j < end; j++)
		{
			prepareCylinder(j);
		}
	}
}

void GuiRibbon::finishUpdate()
{
	forceRender(true, true);
}

void GuiRibbon::updateMultiPositions(Atom *a, WithPos &wp)
{
	updateSinglePosition(a, wp.ave);
}

std::ostream &operator<<(std::ostream &ss, const GuiRibbon::Watch::Entry &entry)
{
	ss << "{" << (!entry.atom ? "null_atom" : entry.atom->desc()) << ", ";
	ss << entry.pos << ", points to idx " << entry.next_idx << "}";
	return ss;
}
