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

#ifndef __vagabond__ProbeBond__
#define __vagabond__ProbeBond__

#include <vagabond/gui/elements/IndexedBatch.h>
#include <vagabond/gui/elements/ButtonResponder.h>
#include <vagabond/core/protonic/Probe.h>

class BondProbe;
class ProbeAtom;
class ProtonNetworkView;
class ProbeBondBatch;

/** One bond in ProtonNetworkView's shared ProbeBondBatch - owns a slot in
 *  it rather than being its own Renderable (see ProbeBondBatch's header
 *  comment for why). */
class ProbeBond : public BatchHandle, public ButtonResponder
{
public:
	ProbeBond(ProtonNetworkView *view, ProbeBondBatch *batch,
	         BondProbe *probe);

	void updateProbe();
	void updatePosition();
	void fullUpdate();

	BondProbe *bondProbe()
	{
		return _probe;
	}

	Probe *probe()
	{
		return _probe;
	}

	bool isSelected()
	{
		return _selected;
	}

	ProtonNetworkView *view()
	{
		return _view;
	}

	// updatePosition() draws between these, not the physics probe
	// position, so a bond always follows wherever its two endpoint dots
	// are currently rendered - physics-synced in 3D mode, PositionShifter-
	// driven in 2D mode - without needing to know which mode it is.
	void setEndpoints(ProbeAtom *left, ProbeAtom *right)
	{
		_leftGui = left;
		_rightGui = right;
	}

	void offerBondMenu();
	virtual void interacted(int idx, bool hover, bool left);
	virtual void selected(int idx, bool inverse);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	ProbeBondBatch *_batch = nullptr;
	size_t _slot = 0;

	BondProbe *_probe = nullptr;
	ProtonNetworkView *_view = nullptr;

	ProbeAtom *_leftGui = nullptr;
	ProbeAtom *_rightGui = nullptr;

	bool _selected{};
};

#endif
