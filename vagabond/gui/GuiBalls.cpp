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

#include "GuiBalls.h"
#include "GuiBond.h"
#include "Display.h"

#include <vagabond/gui/elements/FloatingText.h>

#include <vagabond/core/matrix_functions.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include "VagWindow.h"

#include <iostream>
#include <thread>

GuiBalls::GuiBalls(GuiAtom *parent) : GuiRepresentation(parent)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/simple_point.vsh");
	setFragmentShaderFile("assets/shaders/simple_point.fsh");
	setImage("assets/images/blob.png");
	
	_template = new SimplePolygon();
	_template->addVertex(glm::vec3(0.f));
	_template->addIndex(-1);
	_template->setColour(1., 1., 1.);
	
	_renderType = GL_POINTS;
	_bonds = new GuiBond();
	_bonds->setDisabled(true);
	setName("Gui balls");

	_size *= Window::ratio() / 2;
#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
}

void GuiBalls::click(bool left)
{
	interacted(currentVertex(), false, left);
}

bool GuiBalls::mouseOver()
{
	interacted(currentVertex(), true, true);
	return (currentVertex() >= 0);
}

void GuiBalls::unMouseOver()
{
	interacted(-1, true, true);
}


GuiBalls::~GuiBalls()
{
	delete _bonds;
	_bonds = nullptr;
	delete _template;
	_template = nullptr;
	
}

void GuiBalls::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		_vertices[i].extra[0] = offset + 1.5 + i;
	}
}

void GuiBalls::interacted(int idx, bool hover, bool left)
{
	if (_lastIdx == idx)
	{
		return;
	}
	
	if (idx < 0)
	{
		removeObject(_text);
		delete _text; _text = nullptr;
	}

	if (_scene && _indexAtom.count(idx))
	{
		Atom *atom = _indexAtom[idx];

		if (hover)
		{
			std::string str = atom->desc();

			FloatingText *ft = new FloatingText(str, 25);
			ft->setPosition(_vertices[idx].pos);
			removeObject(_text);
			delete _text; _text = nullptr;
			addObject(ft);
			_text = ft;
		}
		else
		{
			sendResponse("left_atom", atom);
		}
	}

	_lastIdx = idx;
}

void GuiBalls::extraUniforms()
{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, _size);
}

void GuiBalls::colourByElement(std::string ele)
{
	glm::vec4 colour = glm::vec4(0.5, 0.5, 0.5, 1.);
	if (ele == "O")
	{
		colour = glm::vec4(1.4, 0.5, 0.5, 1.);
	}
	if (ele == "S")
	{
		colour = glm::vec4(1.4, 1.4, 0.5, 1.);
	}
	if (ele == "P")
	{
		colour = glm::vec4(1.4, 0.5, 1.4, 1.);
	}
	if (ele == "H")
	{
		colour = glm::vec4(1.0, 1.0, 1.0, 1.);
	}
	if (ele == "N")
	{
		colour = glm::vec4(0.5, 0.5, 1.4, 1.);
	}

	_template->setColour(colour.x, colour.y, colour.z);
}

void GuiBalls::setPosition(glm::vec3 position)
{
	if (!is_glm_vec_sane(position))
	{
		throw std::runtime_error("position contains nan or vec values");
	}

	_template->setPosition(position);
}

size_t GuiBalls::indicesPerAtom()
{
	return _template->indexCount();
}

size_t GuiBalls::verticesPerAtom()
{
	return _template->vertexCount();
}

void GuiBalls::updateMultiPositions(Atom *a, WithPos &wp)
{
	_bonds->updateAtoms(a, wp);
}


void GuiBalls::setHidden(Atom *a, const bool &hidden)
{
	int idx = _atomIndex[a];
	_vertices[idx].color[3] = hidden ? 0.1 : 1.0;
}

void GuiBalls::updateSinglePosition(Atom *a, glm::vec3 &p)
{
	int idx = _atomIndex[a];
	glm::vec3 last = _atomPos[a];

	glm::vec3 diff = p - last;

	if (!is_glm_vec_sane(diff))
	{
		_vertices[idx].color[3] = 0.2;
		throw std::runtime_error("position contains nan or vec values");
	}

	int end = idx + verticesPerAtom(); 
	std::unique_lock<std::mutex> lock(_vertLock);
	for (size_t j = idx; j < end; j++)
	{
		_vertices[j].pos += diff;
	}
	lock.unlock();

	_atomPos[a] = p;

	if (!_multi)
	{
		_bonds->updateAtom(a, p);
	}
}

void GuiBalls::watchAtom(Atom *a)
{
	long index = _vertices.size();

	colourByElement(a->elementSymbol());
	setPosition(a->initialPosition());

	appendObject(_template);

	_atomPos[a] = a->initialPosition();
	_atomIndex[a] = index;
	_indexAtom[index] = a;
}

void GuiBalls::watchBonds(AtomGroup *ag)
{
	_bonds->watchBonds(ag);
	addObject(_bonds);
}

void GuiBalls::prepareAtomSpace(AtomGroup *ag)
{
	std::unique_lock<std::mutex> lock(_vertLock);
	_vertices.reserve(verticesPerAtom() * ag->size());
	_indices.reserve(indicesPerAtom() * ag->size());
}

void GuiBalls::addVisuals(Atom *a)
{
	int idx = _atomIndex[a];
	
	for (size_t i = idx; i < idx + verticesPerAtom(); i++)
	{
		_vertices[i].color[3] = 1.f;
	}
	
	_bonds->addVisuals(a);
}

void GuiBalls::removeVisuals()
{
	GuiRepresentation::removeVisuals();
	_bonds->removeVisuals();
}

void GuiBalls::setMulti(bool m)
{
	_multi = m;
	if (m)
	{
		GuiRepresentation::removeVisuals();
		_bonds->setDisabled(false);
		_bonds->setAlpha(1.f);
	}
	else
	{
		setAlpha(1.f);
		_bonds->setDisabled(true);
	}

}

void GuiBalls::finishUpdate()
{
	_bonds->forceRender(true, true);
}

void GuiBalls::selected(int idx, bool inverse)
{
	if (_lastIdx == idx)
	{
		return;
	}

	if (_scene && _indexAtom.count(idx))
	{
		Atom *atom = _indexAtom[idx];
		atom->setSelected(!inverse);
		_vertices[idx].extra[1] = inverse ? 0 : 1;
	}
	forceRender(true, false);
}

void GuiBalls::deselect()
{
	for (auto it = _atomIndex.begin(); it != _atomIndex.end(); it++)
	{
		Atom *atom = it->first;
		atom->setSelected(false);
		_vertices[it->second].extra[1] = 0;
	}

	forceRender(true, false);
}
