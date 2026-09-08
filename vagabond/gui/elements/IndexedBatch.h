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

#ifndef __vagabond__IndexedBatch__
#define __vagabond__IndexedBatch__

#include "IndexResponder.h"
#include <vector>

/** What a batch slot forwards picking to - deliberately not IndexResponder
 *  itself (which drags in the whole SimplePolygon/Renderable chain, GL
 *  state included) since a batched slot's handle owns no GL state of its
 *  own anymore; only the IndexedBatch itself is a Renderable. */
class BatchHandle
{
public:
	virtual ~BatchHandle() {}
	virtual void interacted(int idx, bool hover, bool left) {}
	virtual void selected(int idx, bool inverse) {}
};

/** Generic many-slots-one-draw-call batch. Copies a template Renderable's
 *  vertices/indices into a shared buffer once per appendSlot() call (index-
 *  offset adjusted, same trick Renderable::appendObject() uses for actual
 *  child objects), so N logical instances render via a single VAO/program
 *  bind and a single glDrawElements instead of N independent ones.
 *
 *  Picking is preserved: each slot gets exactly one pick index (matching
 *  requestedIndices() == slotCount()), stamped into every vertex belonging
 *  to that slot by reindex(), and interacted()/selected() are forwarded to
 *  whatever BatchHandle was registered for that slot - so a slot can still
 *  behave like an independent interactive object without being its own
 *  Renderable.
 *
 *  This does not replace GuiBalls/GuiBond, which predate this class and
 *  solve the same problem bespoke for the main protein ball-and-stick
 *  view - see the ProtonNetworkView rendering-performance plan for the
 *  intended follow-up to move them onto this shared implementation once
 *  it has proven itself on ProbeAtom/ProbeBond/ProbeCharge. */
class IndexedBatch : public IndexResponder
{
public:
	/** @param unitVertices/unitIndices the vertices/indices copied per
	 *  slot (indices relative to unitVertices, e.g. a 4-vertex quad's
	 *  {0,1,2, 1,2,3}). Shader files, texture, usesProjection and
	 *  renderType are the subclass's own responsibility, same as any
	 *  other Renderable.
	 *  @param pickComponent which component of Snow::Vertex::extra the
	 *  fragment shader reads back as the GPU pick id (matches the
	 *  existing per-class convention - e.g. 3 for anything using
	 *  indexed_box.fsh/axes.vsh, as ProbeAtom/ProbeBond/ProbeCharge
	 *  currently stamp by hand in their own reindex() overrides). */
	IndexedBatch(const std::vector<Snow::Vertex> &unitVertices,
	            const std::vector<GLuint> &unitIndices,
	            int pickComponent = 3);

	/** Appends one slot's worth of vertices/indices (copied from the
	 *  unit template) and returns its slot id. Optionally registers the
	 *  interactive handle for that slot up front; setHandle() can also
	 *  be called later. */
	size_t appendSlot(BatchHandle *handle = nullptr);

	void setHandle(size_t slot, BatchHandle *handle);

	size_t slotCount() const
	{
		return _handles.size();
	}

	size_t verticesPerSlot() const
	{
		return _vertsPerSlot;
	}

	size_t indicesPerSlot() const
	{
		return _idxPerSlot;
	}

	/** first vertex belonging to this slot; the slot occupies
	 *  [vertexOffset(slot), vertexOffset(slot) + verticesPerSlot()). */
	size_t vertexOffset(size_t slot) const
	{
		return slot * _vertsPerSlot;
	}

	virtual size_t requestedIndices()
	{
		return _handles.size();
	}

	virtual bool selectable() const
	{
		return true;
	}

	virtual void reindex();
	virtual void interacted(int idx, bool hover, bool left);
	virtual void selected(int idx, bool inverse);
private:
	size_t _vertsPerSlot = 0;
	size_t _idxPerSlot = 0;
	int _pickComponent = 3;

	std::vector<Snow::Vertex> _unitVertices;
	std::vector<GLuint> _unitIndices;

	std::vector<BatchHandle *> _handles;
};

#endif
