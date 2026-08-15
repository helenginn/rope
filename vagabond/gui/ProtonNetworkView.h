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
#include <memory>
#include <atomic>

class PositionShifter;
class HydrogenProbe;
class ProbeCharge;
class CliqueView;
class ProbeBond;
class ProbeAtom;
class BondProbe;
class AtomProbe;
class TextButton;
class Model;
class Probe;
class Menu;

class ProtonNetworkView : public Mouse3D, public Responder<Probe>,
public IndexResponseView
{
public:
	ProtonNetworkView(Scene *scene, Model *model);
	~ProtonNetworkView();

	Network &network()
	{
		return *_network;
	}

	// (re)builds _network from scratch on a background thread - title and
	// back button only until it finishes, same treatment whether this is
	// the first build (called from setup()) or a rebuild requested via
	// EditModel's "Recalculate proton network" (see its own comment for
	// why a pH/pKa change needs a full rebuild rather than a lighter
	// update). Safe to call again while a previous build/rebuild is still
	// in flight - the old one's completion job checks the same
	// _buildCancelled flag this bumps first and quietly does nothing.
	void buildNetwork();

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
	virtual void sendClickSelection(double x, double y, bool inverse);
	void expandSelectionToNeighbours(int max_jumps = INT_MAX);
	void completeResidues(bool stop_at_alpha = false);
	void selectProbes(const OpSet<Probe *> &probes, bool on = true);
	void arrangeFigure();
	void leave2D();
	void cancelAnalysis();

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
	void ensureCliqueView();
	void showCliqueView();
	void makeNewClique();
	void askForSelectionPlan();
	void selectUsingPlan(std::string plan);

	// tears down every renderable that points into the current _network
	// (probe/bond/charge renderables, the clique view, the 2D layout
	// shifter) and deletes _network itself - everything findAtomProbes()
	// builds, undone. Immediate (not deleteLater()) deletion throughout,
	// same ordering constraint the old destructor already documented:
	// these renderables hold raw pointers into _network's Probes and must
	// be gone before it is. Leaves the title/back button/menu button
	// alone - unlike deleteObjects(), this only touches network-derived
	// state, so it is also what a live rebuild (not just final teardown)
	// needs.
	void clearNetworkObjects();

	std::map<Probe *, ProbeAtom *> _textProbes;
	std::map<Probe *, ProbeBond *> _bondProbes;
	std::map<Probe *, ProbeCharge *> _countProbes;
	
	OpSet<Probe *> _allProbes;
	OpSet<Probe *> _hProbes;

	PositionShifter *_shifter{};

	// per-atom "reach" for 2D layout - which other atoms (1 or 2 covalent
	// bonds away, see arrangeFigure()) get a real distance-matching spring
	// force from _shifter, via setWeightFn(), rather than PositionShifter's
	// own now-removed limitSensitivity()/relay mechanism. Owned here
	// rather than by PositionShifter since it's specific to this view's
	// bond-graph notion of "related", not something PositionShifter itself
	// should know about. Cleared in leave2D() alongside _shifter itself,
	// since entries are ProbeAtom* pointers only valid for the 2D session
	// that built them.
	std::map<void *, OpSet<void *>> _reach;

	ProbeAtom *_manual{};

	Model *_model = nullptr;
	Network *_network = nullptr;

	// set (to a fresh, independent flag) each time buildNetwork() starts
	// a background build - the background thread's completion job and
	// this view's own destructor/next buildNetwork() call both check the
	// specific flag *that build* captured, so an in-flight build whose
	// result is no longer wanted (view destroyed, or superseded by
	// another rebuild) can be told to discard itself instead of touching
	// a _network that has since moved on - same shared_ptr<atomic<bool>>
	// idiom ViewCorrelations::viewAll() uses for its own cancellable
	// background assembly.
	std::shared_ptr<std::atomic<bool>> _buildCancelled;

	ProbeAtom *_activeProbe = nullptr;
	Renderable *_active = nullptr;
	CliqueView *_cv = nullptr;

	// created (or re-created, on a rebuild) by makeMainMenu() - the only
	// piece of findAtomProbes()/makeMainMenu()'s output that isn't
	// already tracked in one of the maps/sets above, so clearNetworkObjects()
	// needs its own handle to remove it too.
	TextButton *_menuButton = nullptr;

	Clique *_activeClique = nullptr;
	
	std::function<void()> _onClick{};
	bool _analysing = false;
};

#endif
