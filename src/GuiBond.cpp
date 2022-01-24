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
#include "Atom.h"
#include "BondLength.h"
#include "AtomGroup.h"
#include <iostream>

typedef std::map<Atom *, glm::vec3> AtomPosMap;

GuiBond::GuiBond() : Renderable()
{
	_renderType = GL_LINES;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/box.fsh");
}

void GuiBond::updateAtom(Atom *a, glm::vec3 pos)
{
	if (_atomIdx.count(a) == 0)
	{
		_atomIdx[a] = vertexCount();
		addVertex(pos);
	}
	
	size_t idx = _atomIdx[a];
	_vertices[idx].pos = pos;
}

void GuiBond::watchBonds(AtomGroup *a)
{
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
		
		_indices.push_back(start_idx);
		_indices.push_back(end_idx);
	}
}
