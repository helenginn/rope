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

#include "Points.h"
#include "Visual.h"
#include "Untangle.h"
#include <vagabond/core/Atom.h>
#include <vagabond/core/BondLength.h>
#include <vagabond/gui/elements/Window.h>

Points::Points(Visual *visual)
{
	_visual = visual;
	_untangle = _visual->untangle();

	_renderType = GL_POINTS;
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/simple_point.vsh");
	setFragmentShaderFile("assets/shaders/simple_point.fsh");
	setImage("assets/images/blob.png");

	_size = 15 * Window::ratio() / 2;
}

void Points::colourAtom(Vertex &vertex, const std::string &conf)
{
	glm::vec4 colour = _visual->colourFor(conf);
	glm::vec4 grey = glm::vec4(0.2, 0.2, 0.2, 0.);
	vertex.color = grey + colour * 2.f;
	vertex.color[3] = 1;
}

void Points::repositionAtoms()
{
	for (auto atomConf : _atoms)
	{
		repositionAtom(atomConf.first);
	}
	
	forceRender(true, false);
}

void Points::repositionAtom(Atom *atom)
{
	std::vector<int> &indices = _map[atom];

	for (const int &idx : indices)
	{
		const std::string &conf = _atoms[idx].second;
		glm::vec3 tmp = atom->conformerPositions()[conf].pos.ave;
		_vertices[idx].pos = tmp;
	}
}

void Points::addAtom(Atom *atom)
{
	OpSet<std::string> confs = atom->conformerList();

	for (const std::string &conf : confs)
	{
		glm::vec3 p = atom->conformerPositions()[conf].pos.ave;
		
		Vertex &vert = addVertex(p);
		colourAtom(vert, conf);
		addIndex(-1);
		
		_map[atom].push_back(_atoms.size());
		_atoms.push_back({atom, conf});
	}
}

void Points::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		_vertices[i].extra[0] = offset + i + 1.5;
	}
}

void Points::switchConfs(Atom *a, const std::string &l, const std::string &r,
                         bool one_only)
{
	auto per_job = [this](Atom *a) { repositionAtom(a); };
	
	
	_untangle->switchConfs(a, l, r, one_only, per_job);
//	_untangle->backgroundUntangle({a});

	_visual->updateBonds();

	forceRender(true, true);
}

void Points::interacted(int idx, bool hover, bool left)
{
	if (!hover)
	{
		std::pair<Atom *, std::string> pair = _atoms[idx];
		Atom *atom = pair.first;

		OpSet<std::string> confs = atom->conformerList();
		if (confs.size() == 2)
		{
			std::string first = *confs.begin();
			std::string second = *(++confs.begin());
			switchConfs(atom, first, second, !left);
		}
	}
}

void Points::extraUniforms()
{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, _size);
}

