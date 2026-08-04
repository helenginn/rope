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

#include <vagabond/gui/BondRod.h>
#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/core/protonic/Probe.h>

class BondProbe;
class ProbeAtom;
class ProtonNetworkView;

class ProbeBond : public BondRod, public ButtonResponder,
virtual public IndexResponder
{
public:
	ProbeBond(ProtonNetworkView *view, BondProbe *probe);

	void updateProbe();
	void updatePosition();
	void fullUpdate();

	virtual size_t requestedIndices();

	BondProbe *bondProbe()
	{
		return _probe;
	}

	Probe *probe()
	{
		return _probe;
	}

	virtual bool selectable() const
	{
		return true;
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

	void reindex();
	void interacted(int idx, bool hover, bool left);
	void offerBondMenu();
	virtual void selected(int idx, bool inverse);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	BondProbe *_probe = nullptr;
	ProtonNetworkView *_view = nullptr;

	ProbeAtom *_leftGui = nullptr;
	ProbeAtom *_rightGui = nullptr;

	bool _selected{};
};

#endif
