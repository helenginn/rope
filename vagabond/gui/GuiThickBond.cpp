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

#include "GuiThickBond.h"
#include "BondLength.h"
#include <vagabond/core/AtomGroup.h>

GuiThickBond::GuiThickBond(GuiAtom *parent) : GuiRepresentation(parent)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	setImage("assets/images/bond.png");
}

GuiThickBond::~GuiThickBond()
{

}

void GuiThickBond::updateSinglePosition(Atom *a, glm::vec3 &p)
{
	std::vector<size_t> &idxs = _atom2Markers[a];

	for (size_t i = 0; i < idxs.size(); i++)
	{
		AtomMarker &am = _markers[idxs[i]];

		int offset = 0;
		if (am.right == a)
		{
			offset = 1;
		}
		
		int first = am.idx + offset;
		_vertices[first].pos = p;
		int second = first + 2;
		_vertices[second].pos = p;
		
		glm::vec3 dir = _vertices[am.idx + 1].pos - _vertices[am.idx].pos;
		for (size_t j = 0; j < 4; j++)
		{
			_vertices[am.idx + j].normal = dir;
		}
	}
}

void GuiThickBond::updateMultiPositions(Atom *a, Atom::WithPos &wp)
{
	updateSinglePosition(a, wp.ave);
}

void GuiThickBond::prepareAtomSpace(AtomGroup *ag)
{
	lockMutex();
	_vertices.reserve(verticesPer() * ag->bondLengthCount());
	_indices.reserve(indicesPer() * ag->bondLengthCount());
	_markers.reserve(ag->bondLengthCount());
	unlockMutex();
}

void GuiThickBond::addBond(Atom *left, Atom *right)
{
	glm::vec3 start = left->initialPosition();
	glm::vec3 end = right->initialPosition();
	glm::vec3 dir = end - start;

	AtomMarker marker{left, right, _vertices.size(), 0};
	addThickLine(start, dir);
	
	_atom2Markers[left].push_back(_markers.size());
	_atom2Markers[right].push_back(_markers.size());
	_markers.push_back(marker);
}

void GuiThickBond::watchAtomGroup(AtomGroup *ag)
{
	for (size_t i = 0; i < ag->bondLengthCount(); i++)
	{
		BondLength *bl = ag->bondLength(i);
		
		Atom *left = bl->atom(0);
		Atom *right = bl->atom(1);
		
		addBond(left, right);
	}
}

void GuiThickBond::addVisuals(Atom *a)
{
	/* make opaque */
	std::vector<size_t> &idxs = _atom2Markers[a];

	for (size_t i = 0; i < idxs.size(); i++)
	{
		AtomMarker &am = _markers[idxs[i]];
		am.opaque_request++;

		for (size_t j = 0; j < 4 && am.opaque_request > 1; j++)
		{
			_vertices[am.idx + j].color[3] = 0.f;
		}
	}
}

void GuiThickBond::removeVisuals()
{
	setAlpha(-1.f);
	for (AtomMarker &am : _markers)
	{
		am.opaque_request = 0;
	}
}

