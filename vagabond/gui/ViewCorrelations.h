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

#ifndef __vagabond__ViewCorrelations__
#define __vagabond__ViewCorrelations__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/Mouse2D.h>
#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/Item.h>
#include <vagabond/core/ResidueRange.h>
#include <vagabond/core/protonic/hnet.h>
#include <vagabond/utils/Eigen/Dense>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <list>
#include <vector>

class Correlative;
class Probe;
class Clique;
class LineGroup;
class ClusterPlot;
class HBondDiagram;
class MatrixPlot;
class TextButton;

class ViewCorrelations : public Mouse2D, public IndexResponseView
{
public:
	ViewCorrelations(Scene *prev, Clique *clique);
	~ViewCorrelations();

	virtual void setup();
	virtual void sendSelection(float t, float l, float b, float r,
	                           bool inverse);
	virtual void sendClickSelection(double x, double y, bool inverse);

	// HBondDiagram atoms are draggable (Renderable::setDragFunction() -
	// see HBondDiagram::makeAtoms()'s own comment); while one is actively
	// being dragged (_dragged, set by the base Scene::mousePressEvent),
	// this must skip Mouse2D's own unconditional 2D-pan handling (which
	// would otherwise also fire on the very same left-drag gesture and
	// fight the atom's own movement) - same idiom as ProtonNetworkView::
	// mouseMoveEvent's own _manual check, just against the generic
	// _dragged the base Renderable/Scene drag mechanism itself sets,
	// since HBondDiagram is not otherwise an interactive/selectable view
	// the way ProtonNetworkView's whole figure is.
	virtual void mouseMoveEvent(double x, double y);

	// Mouse2D::mouseReleaseEvent() skips its own trailing
	// interpretMouseButton(button, false) call whenever a modal is
	// already showing by the time it gets there - true here specifically
	// because showStateContextMenu() opens one from within the very same
	// call, one layer further down (Scene::mouseReleaseEvent() ->
	// checkIndexBuffer() -> interactedWithNothing()), on the plain right
	// click that also triggers it. Left uncorrected, _right stays stuck
	// true from that click's own press event onward - see the .cpp for
	// the knock-on effect.
	virtual void mouseReleaseEvent(double x, double y,
	                               SDL_MouseButtonEvent button);

	// right-click-in-white-space handler (IndexResponseView's own hook,
	// fired by checkIndexBuffer() whenever a click lands on no indexed
	// object at all - see ProtonNetworkView::interactedWithNothing() for
	// the same idiom) - builds the state-clustering "Invert selection" /
	// "Exclude unselected" / "Reset selection" context menu (see
	// showStateContextMenu()) on a plain right click (!left) while
	// _subnetworkView is StateClustering; a no-op otherwise, so left-click
	// deselect-everything and every other tab's own plain-click behaviour
	// (currently none) are unaffected.
	virtual void interactedWithNothing(bool left, bool hover = false);

	// shared tail of both overrides above - see _lastSelectedStates's own
	// comment.
	void captureStateSelection();
	void makeList();
	void viewSubnetwork(Clique &clique);
	void occupancies();
	void viewAll();
private:
	// which of a subnetwork's views is currently showing - persists as
	// the default for the next subnetwork clicked, not reset per-click.
	enum class SubnetworkView
	{
		CorrelationMatrix,
		StateClustering,
		HydrogenBondDiagram,
	};

	// builds the "Correlation matrix" / "State clustering" /
	// "Hydrogen-bonding diagram" toggle directly underneath the
	// subnetwork name set by setInformation() in viewSubnetwork() - the
	// currently-active mode's button is shown inert (Button::setInert
	// (true, true)) as the "you are here" indicator, matching the toggle
	// idiom already used in DisplayOptions::refresh().
	void makeSubnetworkViewToggle();

	// dispatches to showCorrelationMatrix()/showStateClustering()/
	// showHydrogenBondDiagram() per _subnetworkView.
	void showSubnetworkView(Clique &clique);

	// the pre-existing aligned-MatrixPlot-grid view (all of the old
	// viewSubnetwork() body).
	void showCorrelationMatrix(Clique &clique);

	// clusters this subnetwork's own CertainStates by per-state distance
	// (see CertainStates::distance()/distanceMatrix()), sized by each
	// state's Boltzmann weight (probsForLocalAve()). Only ever plots
	// _plottedStateIndices's own states (see its own comment) - every
	// state unless "Exclude unselected" (see showStateContextMenu()) has
	// narrowed that down.
	void showStateClustering(Clique &clique);

	// builds and shows the right-click-in-white-space menu for the
	// state-clustering plot - see interactedWithNothing(). Three actions,
	// all acting on _viewedSubnetwork (rebuilding showStateClustering()
	// afterwards where the plot itself needs to change):
	//  - "Invert selection": flips every currently-plotted point's own
	//    highlight (captureStateSelection() syncs _lastSelectedStates
	//    afterwards) - a pure ClusterPlot-local toggle, no rebuild.
	//  - "Exclude unselected": gates the plot down to just whatever is
	//    currently highlighted - sets _includedStates to (a fresh capture
	//    of) _lastSelectedStates, a no-op if nothing is highlighted, then
	//    clears _lastSelectedStates and rebuilds.
	//  - "Reset selection": undoes any previous gate (_includedStates.
	//    clear()), clears _lastSelectedStates, and rebuilds.
	void showStateContextMenu();

	// see showStateContextMenu()'s own comment.
	void invertStateSelection();
	void excludeUnselectedStates();
	void resetStateSelection();

	// colour-swatch + meaning legend down the side of the state
	// clustering plot, for whichever colours showStateClustering() just
	// used - only called when a watched signal was actually found and
	// coloured (see its own call site). type picks the ExistenceType
	// (Absent/Present) vs BondType (Absent/Acceptor/Donor) wording, and
	// always includes the default (unassigned) colour/meaning too.
	// labelSums (index 0 = Unassigned, 1..3 = label 0..2) is each
	// option's total probability mass, shown as a percentage of
	// totalProbSum (the sum over every state, so all entries add to
	// 100%).
	void makeStateClusteringLegend(Probe *probe, hnet::Types type,
	                               const std::vector<float> &labelSums,
	                               float totalProbSum);

	// 2D chemical-diagram-style layout (see HBondDiagram) of every node
	// this subnetwork's CertainStates touches (CertainStates::ptps() -
	// atoms plus the non-covalent bonds between them; ptps() only ever
	// holds non-covalent bonds/uncertain atoms in the first place, see
	// ExhaustiveSearch, so this is naturally a hydrogen-bonding diagram
	// without needing to filter out the covalent skeleton itself).
	// Independent of any live ProtonNetworkView - see HBondDiagram's own
	// header comment for why that is safe even over overlapping atoms.
	void showHydrogenBondDiagram(Clique &clique);

	SubnetworkView _subnetworkView = SubnetworkView::HydrogenBondDiagram;

	// non-owning - the subnetwork last opened via viewSubnetwork(), so the
	// view-mode toggle buttons can redraw it when clicked without needing
	// the clique passed back in from outside.
	Clique *_viewedSubnetwork{};

	// which of the top-level (whole-clique, viewAll()) views is currently
	// showing - persists as the default for the next time this clique is
	// (re-)viewed, mirroring _subnetworkView above.
	enum class TopLevelView
	{
		CorrelationMatrix,
		SubnetworkClustering,
	};

	// builds the "Correlation matrix" / "Subnetwork clustering" toggle for
	// the top-level (viewAll()) screen - shown from the moment stage 1
	// (assembly) starts, not gated behind full completion, since stage 1
	// is safely cancellable (same _cancelled flag the back button uses).
	// "Subnetwork clustering" is temporarily disabled only while stage 2
	// (FloydWarshall gap-filling) is running, since that has no
	// cancellation support - see the .cpp for why that specific window
	// is unsafe to switch away from.
	//
	// Idempotent - safe to call again later purely to refresh which
	// button looks active/enabled (e.g. once stage 2 finishes and
	// "Subnetwork clustering" should re-enable), without an intervening
	// deleteTemps(): cleans up whichever pair it previously built itself
	// (_correlationMatrixButton/_subnetworkClusteringButton) first.
	// Callers that deleteTemps() the whole screen first (viewAll(),
	// showTopLevelView(), viewSubnetwork()) null those two members out
	// themselves right after, since deleteTemps() only queues the
	// underlying Renderables for deletion without telling this class -
	// leaving them set would make this function try to double-delete
	// buttons deleteTemps() already queued.
	void makeTopLevelViewToggle();

	// deleteTemps() + rebuild whichever top-level view is now active. If
	// switching to CorrelationMatrix and a previous run already fully
	// completed (_matrixComplete), reuses that cached _matrix/_correlative
	// via buildCorrelationMatrixUI() instead of re-running the whole
	// assembly.
	void showTopLevelView();

	// (re)builds the MatrixPlot, and (unless addButton is false) the
	// "Communication analysis" button, from the current _matrix/
	// _correlative/_result - shared by finishAssembly() (first run,
	// addButton=false - see its own comment for why) and
	// showTopLevelView()'s cached-reuse path (subsequent switches back
	// from subnetwork clustering, where the matrix is already known
	// complete and the button belongs alongside it immediately).
	MatrixPlot *buildCorrelationMatrixUI(bool addButton = true);

	// the "Communication analysis" button alone - split out of
	// buildCorrelationMatrixUI() so finishAssembly() can add it once
	// FloydWarshall (fill_gaps) actually finishes, rather than the
	// moment the (still gap-filling, not yet analysable) matrix first
	// appears.
	void addCommunicationAnalysisButton();

	// one node per subdivision of _clique with a CertainStates, distance
	// derived from how many nodes (CertainStates::ptps(), not just each
	// subnetwork's own core probes - ptps() already includes whatever
	// peripheral nodes that subnetwork's own analysis pulled in, which is
	// what makes this accurate) two subnetworks have in common: more
	// shared nodes -> lower distance. Mirrors showStateClustering()'s
	// use of ClusterPlot, one level up (subnetworks instead of states).
	void showSubnetworkClustering();

	// shared by showStateClustering() and showSubnetworkClustering():
	// positions a freshly-built ClusterPlot at this view's standard
	// content spot, registers it for shift+drag/shift+click selection
	// (replacing any previous plot's own registration - see the .cpp),
	// and starts its physics.
	void placeClusterPlot(ClusterPlot *cp);

	// deleteTemps() alone queues a ClusterPlot's underlying Renderable
	// for deletion but does not deregister it from IndexResponseView's
	// own _responders list (IndexResponder::~IndexResponder() never
	// self-deregisters) - a shift+click landing on whatever view replaced
	// it (e.g. HydrogenBondDiagram, which never calls placeClusterPlot()
	// itself) would then walk a dangling pointer into the just-deleted
	// plot and crash. Every deleteTemps() call in this class must go
	// through here instead, which also clears _activeClusterPlot for the
	// same reason (see its own comment).
	void clearScreen();

	// signals _cancelled (if set) and then joins _stage2Thread if it is
	// still running - called from clearScreen() (so nothing deleteTemps()s
	// a MatrixPlot stage 2 still holds a raw pointer to) and from the
	// destructor (so the same is true if the whole Scene is being torn
	// down instead, e.g. via the "back" button, which used to just flag
	// cancellation and move straight on to deleting the Scene without
	// waiting for the background thread to actually notice).
	void cancelStage2();

	// non-owning - whichever ClusterPlot placeClusterPlot() last
	// registered as an IndexResponder, purely so a later call can tell
	// clearResponders() to drop it before the temp object itself gets
	// queued for deletion by the next deleteTemps().
	ClusterPlot *_activeClusterPlot = nullptr;

	// updated (from _activeClusterPlot->selectedIndices()) whenever a
	// selection is made while _subnetworkView is StateClustering - see
	// sendSelection()/sendClickSelection() - so showHydrogenBondDiagram()
	// still has the selection available after switching tabs, even though
	// deleteTemps() (viewSubnetwork()) will have already destroyed the
	// ClusterPlot itself (and left _activeClusterPlot dangling) by the
	// time that happens. Always a true CertainStates state index (never a
	// plot-local one - see _plottedStateIndices) even when _includedStates
	// has narrowed the plot down to fewer than state_count() points.
	// Empty means "no selection" - showHydrogenBondDiagram() then falls
	// back to _includedStates (or, if that's empty too, every state).
	std::vector<int> _lastSelectedStates;

	// "Exclude unselected" (showStateContextMenu()) narrows the working
	// set of states down to these true state indices; empty means every
	// state is still in play (the default, and what "Reset selection"
	// restores). Unlike _lastSelectedStates (a transient highlight that
	// switching tabs and back should not lose but does not otherwise
	// persist), this is a hard gate - showStateClustering() only ever
	// plots these states, and showHydrogenBondDiagram() never counts a
	// gated-out one even when _lastSelectedStates is empty.
	std::vector<int> _includedStates;

	// showStateClustering()'s own plot-local index -> true state index
	// mapping for whatever it last built (== _includedStates in that same
	// order, or identity 0..state_count()-1 if _includedStates is empty) -
	// ClusterPlot has no notion of what its own node indices represent
	// (see its own class comment), so this is what translates its
	// selectedIndices() back to true state indices (captureStateSelection())
	// and _lastSelectedStates back to plot-local ones (restoring highlight
	// after a rebuild).
	std::vector<int> _plottedStateIndices;

	TopLevelView _topLevelView = TopLevelView::CorrelationMatrix;

	// non-null only between a makeTopLevelViewToggle() call and whatever
	// next wipes the screen (deleteTemps()) - see that method's own
	// comment.
	TextButton *_correlationMatrixButton = nullptr;
	TextButton *_subnetworkClusteringButton = nullptr;

	// second half of viewAll() - runs once the (cancellable) assembly of
	// _result/_correlative below has completed on the main thread.
	void finishAssembly();

	// builds the search.png icon in the top-right corner and wires up the
	// modal/cross-to-clear/refresh behaviour, all as closures local to this
	// call (see the .cpp) - lg is captured rather than stashed in a member,
	// since nothing outside this wiring ever needs to reach it again. The
	// query text itself is persisted on _clique (Clique::searchText()) so
	// it survives destroying and reconstructing this whole Scene, not
	// just this method's own closure - see that method's own comment.
	void makeSearchButton(LineGroup *lg);

	// hides (Item::setHidden()) every subdivision of clique - and recurses
	// into their own subdivisions, since the list is a full tree - whose
	// probes don't touch any of tokens; un-hides the rest. Empty tokens
	// means "show everything". Expects tokens to already have gone through
	// resolveChains() - it does no chain defaulting/correction itself.
	void filterSubdivisions(Clique &clique,
	                        const std::vector<ResidueRangeToken> &tokens);

	bool cliqueMatchesTokens(Clique &clique,
	                         const std::vector<ResidueRangeToken> &tokens);

	// fills in each token's chain against the chains actually present
	// among _clique's probes: a token with no chain prefix (e.g. plain
	// "115") gets the alphabetically-first chain that actually has a
	// residue in that token's range - not just the alphabetically-first
	// chain outright, which may be e.g. a water chain that never reaches
	// residue 115 at all. A token whose chain doesn't exist but matches
	// an existing one case-insensitively (e.g. user typed "a115",
	// structure only has chain "A") gets corrected to that chain's actual
	// casing, rather than silently matching nothing.
	std::vector<ResidueRangeToken>
	resolveChains(const std::vector<ResidueRangeToken> &tokens);

	Clique *_clique{};
	Correlative *_correlative{};

	// shared with whichever background assembly is currently in flight (if
	// any), so setBackJob() can flag it to stop early without needing to
	// touch anything Scene-specific from that background thread.
	std::shared_ptr<std::atomic<bool>> _cancelled;

	// true from the moment viewAll() kicks off a background assembly
	// until that assembly's last exit point (finishAssembly(), or one of
	// the cancelled-early returns) - guards against a second overlapping
	// run, e.g. from clicking the top clique's list entry while the
	// auto-triggered assembly from setup() is still in flight.
	bool _assembling = false;

	// true once finishAssembly()'s FloydWarshall pass has fully finished
	// for the current _matrix/_correlative/_result - lets
	// showTopLevelView() reuse them instantly instead of re-running the
	// whole assembly. Reset false at the start of every fresh viewAll()
	// run, since _result is about to be replaced and won't have been
	// gap-filled yet.
	bool _matrixComplete = false;

	// owns finishAssembly()'s "fill_gaps" background thread directly
	// (rather than the fire-and-forget detached DoJob used elsewhere in
	// this class) specifically so cancelStage2() can join it - i.e. block
	// until the thread has genuinely stopped touching the MatrixPlot* it
	// was given, not just until cancellation has been requested.
	std::thread _stage2Thread;

	// separate from _cancelled (which is reassigned to a fresh flag by
	// every viewAll() run, and is also legitimately set true for a
	// still-alive Scene that just navigated elsewhere) - this one is only
	// ever flipped false, once, in the destructor, and lets a completion
	// job queued by fill_gaps/assemble but not yet drained by the time
	// this Scene is destroyed detect that and skip touching `this`
	// instead of running against freed memory.
	std::shared_ptr<bool> _alive = std::make_shared<bool>(true);

	// makeList() borrows _clique's and its subdivisions' select jobs
	// (shared Item state, not view-local) for the duration this view is
	// open - saved here before being overwritten, and restored in the
	// destructor so nothing is left pointing at this Scene once it closes.
	std::map<Clique *, std::function<void(bool)>> _prevJobs;

	Eigen::MatrixXf _matrix{};
	std::mutex _mutex{};
	Eigen::MatrixXf _result{};

	// viewSubnetwork()'s grid of per-pair MatrixPlots each hold a live
	// reference into one of these - a std::list so pushing new entries
	// during that loop never invalidates references already handed out
	// to earlier MatrixPlots (unlike std::vector, which can reallocate).
	// Cleared alongside deleteTemps() at the top of viewSubnetwork(),
	// once the MatrixPlots referencing the previous run's entries are
	// themselves gone.
	std::list<Eigen::MatrixXf> _subnetworkMats;
};

#endif
