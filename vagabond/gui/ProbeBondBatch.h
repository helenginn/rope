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

#ifndef __vagabond__ProbeBondBatch__
#define __vagabond__ProbeBondBatch__

#include <vagabond/gui/elements/IndexedBatch.h>
#include <vagabond/gui/elements/Library.h>

/** All of ProtonNetworkView's bonds (ProbeBond) in one draw call. Each
 *  bond independently switches which of a small, closed set of images it
 *  shows (weak_bond/strong_bond/lone_pair/.../double_bond - see
 *  BondProbe::display()/CovalentProbe::display()) as the hnet solver
 *  updates it - unlike GuiBond's single fixed line texture, so a plain
 *  shared texture doesn't work here. Instead all of them are packed into
 *  one atlas texture up front (Library::buildAtlas()) and each slot picks
 *  its own sub-rect via assets/shaders_450/axes_atlas.fsh, keyed by the
 *  same image name BondRod/Image::changeImage() would have loaded. */
class ProbeBondBatch : public IndexedBatch
{
public:
	ProbeBondBatch();

	size_t appendBond(BatchHandle *handle);

	/** same quad layout as BondRod::fixVertices() - a line from start to
	 *  start+dir, its width set by axes.vsh from the vertex normal. */
	void fixVertices(size_t slot, const glm::vec3 &start,
	                 const glm::vec3 &dir);

	/** switches which atlas sub-image (see the class comment for the
	 *  full closed set of names) this slot's 4 vertices sample. A name
	 *  outside that set leaves the slot's current image unchanged. */
	void setSlotImage(size_t slot, const std::string &imageName);

	void setSlotColour(size_t slot, double r, double g, double b);
	void setSlotAlpha(size_t slot, double alpha);

	virtual void extraUniforms();
private:
	std::map<std::string, AtlasRect> _rects;
};

#endif
