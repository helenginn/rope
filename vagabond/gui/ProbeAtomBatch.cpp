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

#include "ProbeAtomBatch.h"
#include <vagabond/gui/elements/Window.h>
#include <iostream>

namespace
{

std::vector<Snow::Vertex> unit_atom_vertices()
{
	Snow::Vertex v = {};
	std::vector<Snow::Vertex> vs(4, v);

	// same corners/UVs as Box::addQuad() - vs[k].extra/normal are filled
	// in per-slot by setSlotFull()/changeSlotText().
	vs[0].pos = {-1, -1, 0}; vs[0].tex[0] = 0; vs[0].tex[1] = 1;
	vs[1].pos = {-1, +1, 0}; vs[1].tex[0] = 0; vs[1].tex[1] = 0;
	vs[2].pos = {+1, -1, 0}; vs[2].tex[0] = 1; vs[2].tex[1] = 1;
	vs[3].pos = {+1, +1, 0}; vs[3].tex[0] = 1; vs[3].tex[1] = 0;

	return vs;
}

std::vector<GLuint> unit_atom_indices()
{
	return {0, 1, 2, 1, 2, 3};
}

}

ProbeAtomBatch::ProbeAtomBatch(const std::vector<std::string> &vocabulary,
                              Font::Type type)
: IndexedBatch(unit_atom_vertices(), unit_atom_indices(), 3), _type(type)
{
	setVertexShaderFile("assets/shaders/floating_box_atlas.vsh");
#ifndef __EMSCRIPTEN__
	setFragmentShaderFile("assets/shaders/indexed_box_atlas.fsh");
#else
	setFragmentShaderFile("assets/shaders/box_atlas.fsh");
#endif
	setUsesProjection(true);
	setName("Probe atom batch");

	std::string cacheKey = "probe_atom_atlas_"
	+ std::string(type == Font::Thin ? "thin" : "thick");
	_texid = Library::getLibrary()->buildTextAtlas(cacheKey, vocabulary,
	                                               type, _rects);
}

size_t ProbeAtomBatch::appendAtom(BatchHandle *handle)
{
	size_t slot = appendSlot(handle);
	_slotText.push_back("");
	return slot;
}

const AtlasRect &ProbeAtomBatch::rectFor(const std::string &text)
{
	static AtlasRect identity;

	if (!_rects.count(text))
	{
		std::cout << "ProbeAtomBatch: no atlas entry for '" << text
		<< "'" << std::endl;
		return identity;
	}

	return _rects.at(text);
}

void ProbeAtomBatch::setSlotFull(size_t slot, const std::string &text,
                                float mult, const glm::vec3 &position)
{
	const AtlasRect &r = rectFor(text);

	// same scale Text::makeQuad() bakes into a standalone FloatingText,
	// applied directly to the billboard offset instead of to a
	// then-collapsed position (see FloatingText::correctBox() for why
	// that collapse is a no-op algebraically once done this way).
	float typeFactor = (_type == Font::Thin ? 1.f : 0.7f);
	float m00 = ((float)r.width / 1800.f) * Window::aspect() * typeFactor;
	float m11 = ((float)r.height / 1800.f) * typeFactor;

	static const float bx[4] = {-1, -1, 1, 1};
	static const float by[4] = {-1, 1, -1, 1};

	size_t i = vertexOffset(slot);
	for (size_t k = 0; k < 4; k++)
	{
		Snow::Vertex &v = _vertices[i + k];
		v.extra.x = bx[k] * m00 * mult;
		v.extra.y = by[k] * m11 * mult;
		v.extra.z = 0.f;
		v.pos = position;
		v.normal.x = r.u0;
		v.normal.y = r.u1 - r.u0;
		v.normal.z = r.v1 - r.v0;
	}

	_slotText[slot] = text;

	forceRender(true, false);
}

void ProbeAtomBatch::changeSlotText(size_t slot, const std::string &text)
{
	if (_slotText[slot] == text)
	{
		return;
	}

	const AtlasRect &oldR = rectFor(_slotText[slot]);
	const AtlasRect &newR = rectFor(text);

	float scaleX = (oldR.width > 0) ?
	(float)newR.width / (float)oldR.width : 1.f;
	float scaleY = (oldR.height > 0) ?
	(float)newR.height / (float)oldR.height : 1.f;

	size_t i = vertexOffset(slot);
	for (size_t k = 0; k < verticesPerSlot(); k++)
	{
		Snow::Vertex &v = _vertices[i + k];
		v.extra.x *= scaleX;
		v.extra.y *= scaleY;
		v.normal.x = newR.u0;
		v.normal.y = newR.u1 - newR.u0;
		v.normal.z = newR.v1 - newR.v0;
	}

	_slotText[slot] = text;

	forceRender(true, false);
}

void ProbeAtomBatch::setSlotPosition(size_t slot, const glm::vec3 &position)
{
	size_t i = vertexOffset(slot);

	for (size_t k = 0; k < verticesPerSlot(); k++)
	{
		_vertices[i + k].pos = position;
	}

	forceRender(true, false);
}

glm::vec3 ProbeAtomBatch::slotPosition(size_t slot) const
{
	return _vertices[vertexOffset(slot)].pos;
}

void ProbeAtomBatch::setSlotColour(size_t slot, double r, double g, double b)
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

void ProbeAtomBatch::setSlotAlpha(size_t slot, double alpha)
{
	size_t i = vertexOffset(slot);

	for (size_t j = i; j < i + verticesPerSlot(); j++)
	{
		_vertices[j].color[3] = alpha;
	}

	forceRender(true, false);
}
