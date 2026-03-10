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

#ifndef __vagabond__ProtonNetworkView__
#define __vagabond__ProtonNetworkView__

#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/Responder.h>
#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/utils/OpSet.h>

class PositionShifter;
class HydrogenProbe;
class ProbeBond;
class ProbeAtom;
class BondProbe;
class AtomProbe;
class Probe;
class Menu;

class ProtonNetworkView : public Mouse3D, public Responder<Probe>,
public IndexResponseView
{
public:
	ProtonNetworkView(Scene *scene, Network &network);
	~ProtonNetworkView();
	
	Network &network()
	{
		return _network;
	}

	virtual void setup();
	
	void setActive(Renderable *r)
	{
		if (_active && _active != r)
		{
			_active->setHighlighted(false);
		}
		_active = r;
	}
	
	void setManualAdjust(ProbeAtom *probe);
	
	void setMenu(Menu *menu);
	virtual void keyReleaseEvent(SDL_Keycode pressed);
	virtual void mouseMoveEvent(double x, double y);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);

	virtual void sendSelection(float t, float l, float b, float r, bool inverse);
	void expandSelectionToNeighbours();

private:
	void arrangeFigure();

	virtual void sendObject(std::string tag, void *object);
	virtual void interactedWithNothing(bool left, bool hover);
	void findAtomProbes();
	
	std::map<Probe *, ProbeAtom *> _textProbes;
	std::map<Probe *, ProbeBond *> _bondProbes;
	
	OpSet<Probe *> _allProbes;

	PositionShifter *_shifter{};
	ProbeAtom *_manual{};

	Network &_network;

	ProbeAtom *_activeProbe = nullptr;
	Renderable *_active = nullptr;
};

#endif
