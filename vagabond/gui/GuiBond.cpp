// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "GuiBond.h"

#include <vagabond/core/Atom.h>
#include <vagabond/core/BondLength.h>
#include <vagabond/core/AtomGroup.h>
#include <iostream>

typedef std::map<Atom *, glm::vec3> AtomPosMap;

GuiBond::GuiBond() : Renderable()
{
	_renderType = GL_LINES;
	_copy = new Renderable();

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/box.fsh");
}

GuiBond::~GuiBond()
{
	delete _copy;
	_copy = nullptr;
}

void GuiBond::updateAtom(Atom *a, glm::vec3 pos, int n)
{
	size_t nvtx = _atomIdx.size();
	size_t offset = nvtx * n;

	if (_atomIdx.count(a) == 0)
	{
		return;
	}
	
	size_t idx = _atomIdx[a] + offset;
	_vertices[idx].pos = pos;
	
}

void GuiBond::incrementNetworks(int n)
{
	for (size_t i = _num; i < n; i++)
	{
		appendObject(_copy);
	}

	_num += n;
}

void GuiBond::truncateNetworks(int n)
{
	size_t nidx = _copy->indexCount();
	size_t nvtx = _copy->vertexCount();
	size_t isize = n * nidx;
	size_t vsize = n * nvtx;
	
	_vertices.resize(vsize);
	_indices.resize(isize);
	
	_num = n;
}

void GuiBond::changeNetworks(int n)
{
	if (_num == n)
	{
		return;
	}
	
	if (_num < n)
	{
		incrementNetworks(n);
	}
	else
	{
		truncateNetworks(n);
	}
}

void GuiBond::updateAtoms(Atom *a, Atom::WithPos &wp)
{
	int n = wp.samples.size();
	changeNetworks(n);

	for (size_t i = 0; i < n; i++)
	{
		updateAtom(a, wp.samples[i], i);
	}
	
}

void GuiBond::watchBonds(AtomGroup *a)
{
	int v = 0;
	for (size_t i = 0; i < a->size(); i++)
	{
		Atom *atom = (*a)[i];
		_atomIdx[atom] = v;
		_copy->addVertex(glm::vec3(NAN));
		v++;
	}

	for (size_t i = 0; i < a->bondLengthCount(); i++)
	{
		BondLength *b = a->bondLength(i);

		Atom *start = b->atom(0);
		Atom *end = b->atom(1);
		
		if (_atomIdx.count(start) == 0 || _atomIdx.count(end) == 0)
		{
			continue;
		}
		
		size_t start_idx = _atomIdx[start];
		size_t end_idx = _atomIdx[end];
		
		_copy->addIndex(start_idx);
		_copy->addIndex(end_idx);
	}
	
	appendObject(_copy);
	_num = 1;
}


