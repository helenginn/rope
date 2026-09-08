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
#include <vagabond/core/protonic/hnet.h>

class Probe;
class AtomProbe;
class HydrogenProbe;
class ProtonNetworkView;
class ProbeAtomBatch;

/** One atom/hydrogen label in ProtonNetworkView's shared ProbeAtomBatch -
 *  owns a slot in it rather than being its own Renderable (see
 *  ProbeAtomBatch's header comment for why). */
class ProbeAtom : public BatchHandle, public ButtonResponder
{
public:
	ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
	         AtomProbe *probe);
	ProbeAtom(ProtonNetworkView *view, ProbeAtomBatch *batch,
	         HydrogenProbe *probe);

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
	void hoverOverAtom();
	void offerHeavyAtomMenu();
	void offerHydrogenMenu();
	void declareHydrogen(hnet::Existence::Values value);
	void declareAtomExistence(hnet::Existence::Values value);

	ProbeAtomBatch *_batch = nullptr;
	size_t _slot = 0;

	Probe *_probe = nullptr;

	ProtonNetworkView *_view = nullptr;
	bool _selected{};
};

#endif
