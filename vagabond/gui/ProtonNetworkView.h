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
#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Responder.h>
#include <vagabond/utils/OpSet.h>

class PositionShifter;
class HydrogenProbe;
class ProbeCharge;
class CliqueView;
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
	void setManualAdjust(Probe *probe);
	
	void setMenu(Menu *menu);
	void makeMainMenu();
	void exportHBonds();
	virtual void keyReleaseEvent(SDL_Keycode pressed);
	virtual void keyPressEvent(SDL_Keycode pressed);
	virtual void mouseMoveEvent(double x, double y);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);

	virtual void sendSelection(float t, float l, float b, float r, bool inverse);
	void expandSelectionToNeighbours(int max_jumps = INT_MAX);
	void completeResidues(bool stop_at_alpha = false);
	void selectProbes(const OpSet<Probe *> &probes, bool on = true);
	void arrangeFigure();

	void highlightCliques();

	void setActive(Clique *clique)
	{
		_activeClique = clique;
		_analysing = true;
	}
private:

	void focusOnResidue(std::string chain, int res);
	virtual void sendObject(std::string tag, void *object);
	virtual void interactedWithNothing(bool left, bool hover);
	void findAtomProbes();
	void makeNewClique();
	void linkSymmetricAtomProbes(const hnet::AtomConf &ac);

	
	std::map<Probe *, ProbeAtom *> _textProbes;
	std::map<Probe *, ProbeBond *> _bondProbes;
	std::map<Probe *, ProbeCharge *> _countProbes;
	
	OpSet<Probe *> _allProbes;
	OpSet<Probe *> _hProbes;

	PositionShifter *_shifter{};
	ProbeAtom *_manual{};

	Network &_network;

	ProbeAtom *_activeProbe = nullptr;
	Renderable *_active = nullptr;
	CliqueView *_cv = nullptr;
	
	Clique *_activeClique = nullptr;
	
	std::function<void()> _onClick{};
	bool _analysing = false;
};

#endif
