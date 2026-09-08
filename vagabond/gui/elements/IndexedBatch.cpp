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

#include "IndexedBatch.h"

IndexedBatch::IndexedBatch(Renderable *unitTemplate, int pickComponent)
: _pickComponent(pickComponent)
{
	_vertsPerSlot = unitTemplate->vertexCount();
	_idxPerSlot = unitTemplate->indexCount();

	_unitVertices.reserve(_vertsPerSlot);
	for (size_t i = 0; i < _vertsPerSlot; i++)
	{
		_unitVertices.push_back(unitTemplate->vertex(i));
	}

	_unitIndices = unitTemplate->indices();

	setShadersLike(unitTemplate);
	setUsesProjection(unitTemplate->usesProjection());
	_renderType = unitTemplate->renderType();

	setName("Indexed batch");
}

size_t IndexedBatch::appendSlot(IndexResponder *handle)
{
	size_t slot = _handles.size();
	size_t vOffset = _vertices.size();

	for (size_t i = 0; i < _unitVertices.size(); i++)
	{
		_vertices.push_back(_unitVertices[i]);
	}

	for (size_t i = 0; i < _unitIndices.size(); i++)
	{
		_indices.push_back(_unitIndices[i] + vOffset);
	}

	_handles.push_back(handle);

	return slot;
}

void IndexedBatch::setHandle(size_t slot, IndexResponder *handle)
{
	_handles[slot] = handle;
}

void IndexedBatch::reindex()
{
	size_t offset = indexOffset();

	for (size_t slot = 0; slot < _handles.size(); slot++)
	{
		float pick = offset + 1.5 + slot;
		size_t start = vertexOffset(slot);

		for (size_t i = start; i < start + _vertsPerSlot; i++)
		{
			_vertices[i].extra[_pickComponent] = pick;
		}
	}
}

void IndexedBatch::interacted(int idx, bool hover, bool left)
{
	if (idx < 0 || (size_t)idx >= _handles.size() || _handles[idx] == nullptr)
	{
		return;
	}

	_handles[idx]->interacted(idx, hover, left);
}

void IndexedBatch::selected(int idx, bool inverse)
{
	if (idx < 0 || (size_t)idx >= _handles.size() || _handles[idx] == nullptr)
	{
		return;
	}

	_handles[idx]->selected(idx, inverse);
}
