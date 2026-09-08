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

#ifndef __vagabond__ProbeAtomBatch__
#define __vagabond__ProbeAtomBatch__

#include <vagabond/gui/elements/IndexedBatch.h>
#include <vagabond/gui/elements/Library.h>

/** All of ProtonNetworkView's atom/hydrogen labels (ProbeAtom) in one draw
 *  call. Each label is rendered text (a handful of distinct one-character
 *  states, or a real element symbol for an atom the solver has settled on
 *  as an ion, or rarely a fixed custom string) - packed into one text
 *  atlas up front (Library::buildTextAtlas()) from the full vocabulary
 *  the network could ever need (see ProtonNetworkView::findAtomProbes()),
 *  sound only because an atom that ever displays a real element symbol
 *  never goes back to O/N/S/etc (Helen, on hnet's AtomConnector) - so
 *  there is no case where a slot needs a label outside what was
 *  enumerated at construction.
 *
 *  FloatingText's vertex layout leaves no attribute free for an atlas
 *  sub-rect the way ProbeBondBatch used - extra already holds the
 *  billboard screen-space offset (xyz) and GPU-pick id (w), and tex holds
 *  the plain per-corner quad UV. normal is the one attribute this
 *  pipeline never uses, so the sub-rect (u0, u-span, v-span) lives there
 *  instead - see assets/shaders_450/{floating_box_atlas.vsh,
 *  indexed_box_atlas.fsh}. */
class ProbeAtomBatch : public IndexedBatch
{
public:
	ProbeAtomBatch(const std::vector<std::string> &vocabulary,
	              Font::Type type = Font::Thin);

	size_t appendAtom(BatchHandle *handle);

	/** full rebuild of the slot quad size + position for a given label -
	 *  mirrors FloatingText::setText()+correctBox()+setPosition()
	 *  combined (ProbeAtom::fullUpdateProbe()'s "thorough" path). */
	void setSlotFull(size_t slot, const std::string &text, float mult,
	                 const glm::vec3 &position);

	/** cheaper update for when only the label changes, not size/position
	 *  - mirrors FloatingText::changeText()'s scale-preserving rescale
	 *  (ProbeAtom::updateProbe()'s non-"thorough" path). */
	void changeSlotText(size_t slot, const std::string &text);

	/** moves the slot without touching its size/label - mirrors
	 *  Renderable::setPosition() as FloatingText::updatePosition() used
	 *  it: shift the on-screen dot without resizing/relabelling it. */
	void setSlotPosition(size_t slot, const glm::vec3 &position);

	glm::vec3 slotPosition(size_t slot) const;

	void setSlotColour(size_t slot, double r, double g, double b);
	void setSlotAlpha(size_t slot, double alpha);
private:
	const AtlasRect &rectFor(const std::string &text);

	std::map<std::string, AtlasRect> _rects;
	std::vector<std::string> _slotText;
	Font::Type _type;
};

#endif
