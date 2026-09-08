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

#ifndef __vagabond__ProbeAtom__
#define __vagabond__ProbeAtom__

#include <vagabond/gui/elements/IndexedBatch.h>
#include <vagabond/gui/elements/ButtonResponder.h>
#include <vagabond/gui/elements/FloatingText.h>
#include <vagabond/core/protonic/hnet.h>
#include <functional>

class Probe;
class AtomProbe;
class HydrogenProbe;
class ProtonNetworkView;
class ProbeAtomBatch;
class ProbeAtom;

/** Rendering/picking shell for a "reporter atom" - an AtomProbe built with
 *  a non-empty custom_text (see Coordinated_Core.cpp), giving it its own
 *  unique per-residue label (e.g. "wat-A123") rather than one of the
 *  small fixed states ProbeAtomBatch's text atlas is built from. That
 *  custom text is fixed for the probe's whole lifetime (display() reads
 *  it unconditionally), so which representation a given atom needs is
 *  decided once, at construction, and never changes - see ProbeAtom's
 *  own constructor. There can legitimately be one of these per residue/
 *  water in a large structure, so packing them into one shared atlas
 *  isn't a good fit (see the buildTextAtlas() overflow this replaced);
 *  each one stays its own individual Renderable, exactly as every
 *  ProbeAtom was before this file's batching work. All the actual
 *  interaction/menu logic still lives on the owning ProbeAtom, which
 *  this only forwards picking callbacks to. */
class ProbeAtomStandaloneText : public FloatingText, virtual public IndexResponder
{
public:
	ProbeAtomStandaloneText(ProbeAtom *owner, const std::string &text,
	                       float mult, float yOff);

	virtual size_t requestedIndices()
	{
		return 1;
	}

	virtual bool selectable() const
	{
		return true;
	}

	virtual void reindex();
	virtual void interacted(int idx, bool hover, bool left);
	virtual void selected(int idx, bool inverse);

	// FloatingText and IndexResponder both eventually reach Renderable
	// (through separate, non-virtually-inherited paths - see
	// IndexResponder.h), so any call to a Renderable/FloatingText method
	// is otherwise ambiguous through a ProbeAtomStandaloneText* - these
	// redeclarations resolve it once here (by hiding the ambiguous
	// inherited names with this class's own unambiguous ones) instead of
	// requiring every caller to write out FloatingText::whatever(...).
	void setPosition(glm::vec3 pos)
	{
		FloatingText::setPosition(pos);
	}

	void setColour(double r, double g, double b)
	{
		FloatingText::setColour(r, g, b);
	}

	void setAlpha(double alpha)
	{
		FloatingText::setAlpha(alpha);
	}

	void forceRender(bool vert, bool idx)
	{
		FloatingText::forceRender(vert, idx);
	}

	void setText(const std::string &text)
	{
		FloatingText::setText(text);
	}

	void changeText(const std::string &text)
	{
		FloatingText::changeText(text);
	}

	void correctBox(float mult, float yOffset)
	{
		FloatingText::correctBox(mult, yOffset);
	}

	glm::vec3 centroid()
	{
		return FloatingText::centroid();
	}

	void addMainThreadJob(const std::function<void()> &job)
	{
		FloatingText::addMainThreadJob(job);
	}
private:
	ProbeAtom *_owner;
};

/** One atom/hydrogen label in ProtonNetworkView - a slot in the shared
 *  ProbeAtomBatch for the common case (see that class's header comment),
 *  or its own ProbeAtomStandaloneText for a reporter atom (see that
 *  class's header comment for why those cannot share the batch/atlas).
 *  Either way this owns all the actual domain logic (menus, hnet
 *  callback wiring), not just delegating rendering. */
class ProbeAtom : public BatchHandle, public ButtonResponder
{
public:
	ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
	         AtomProbe *probe);
	ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
	         HydrogenProbe *probe);
	~ProbeAtom();

	void updatePosition();
	void updateProbe();
	void fullUpdate();
	void fullUpdateProbe();

	Probe *probe()
	{
		return _probe;
	}

	bool isSelected() const
	{
		return _selected;
	}

	// current on-screen position - physics-synced in 3D mode,
	// PositionShifter-driven in 2D mode (see ProbeBond::updatePosition(),
	// which follows this rather than the physics probe position
	// directly).
	glm::vec3 currentPosition() const;

	// moves the on-screen dot without resizing/relabelling or resyncing
	// from the physics probe position - PositionShifter's own 2D-layout
	// driver uses this (see ProtonNetworkView::arrangeFigure()/
	// mouseMoveEvent()'s own comments for why plain updatePosition()
	// would undo every drag step).
	void setRenderPosition(const glm::vec3 &pos);
	void setRenderAlpha(double alpha);
	void forceRedraw();

	virtual void interacted(int idx, bool hover, bool left);
	virtual void selected(int idx, bool inverse);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	bool isStandalone() const
	{
		return _standalone != nullptr;
	}

	void hoverOverAtom();
	void offerHeavyAtomMenu();
	void offerHydrogenMenu();
	void declareHydrogen(hnet::Existence::Values value);
	void declareAtomExistence(hnet::Existence::Values value);

	ProbeAtomBatch *_batch = nullptr;
	size_t _slot = 0;

	// only set for a reporter atom (see ProbeAtomStandaloneText) -
	// mutually exclusive with actually holding a slot in _batch.
	ProbeAtomStandaloneText *_standalone = nullptr;

	Probe *_probe = nullptr;

	ProtonNetworkView *_view = nullptr;
	bool _selected{};
};

#endif
