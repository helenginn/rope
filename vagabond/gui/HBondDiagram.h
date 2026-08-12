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

#ifndef __vagabond__HBondDiagram__
#define __vagabond__HBondDiagram__

#include <vagabond/gui/elements/Box.h>
#include <vagabond/utils/OpSet.h>
#include <atomic>
#include <functional>
#include <map>
#include <memory>

class CertainStates;
class PositionShifter;
class Probe;
struct ProbeTypePair;
class Text;

/** Read-only 2D hydrogen-bonding diagram for an arbitrary subset of
 *  Probes (e.g. one subnetwork's CertainStates::ptps()) - same visual
 *  idiom as ProtonNetworkView::arrangeFigure() (PositionShifter-driven,
 *  real 3D Probe::_init distances as the default spring target, 1-2
 *  covalent-bond reach as the spring weight - see makeAtoms()), but
 *  deliberately independent of ProtonNetworkView: it builds its own
 *  plain, non-interactive Text labels and bond-line Images rather than
 *  reusing ProbeAtom/ProbeBond (which are tightly coupled to
 *  ProtonNetworkView's menus/undo stack/live update callbacks). Every
 *  position this displays lives on Renderables this instance itself
 *  constructs, and its PositionShifter is its own independent instance
 *  (no static/shared state anywhere in that chain - see PositionShifter
 *  itself) - so this is safe to construct and run alongside a live
 *  ProtonNetworkView, even over overlapping Probes, without any risk of
 *  the two layouts fighting over the same storage. Mirrors ClusterPlot's
 *  own instantiation idiom (see its header) - deferred start(), position
 *  via setPosition() only. */
class HBondDiagram : public Box
{
public:
	// states/stateWeights (optional): when given, every node's alpha (and
	// a bond's own icon - see computeStateAverages()) reflects the
	// stateWeights-weighted average across states rather than this one-
	// shot live snapshot's actual current value - stateWeights is passed
	// straight through to CertainStates::proportions() (zero out a state's
	// own weight to exclude it, e.g. for an "only these selected states"
	// average), and must be states->state_count() long if given at all.
	// Leaving both at their defaults keeps this exactly the plain live
	// snapshot it always was.
	//
	// positionSource (optional): builds this instance as a "mirror" of
	// another, already-constructed HBondDiagram built from the same
	// nodes (e.g. the complement of a selection - see ViewCorrelations::
	// showHydrogenBondDiagram()) - shares positionSource's own
	// PositionShifter instead of creating its own (so there is exactly
	// one physics thread between the two, not a second, mostly-idle one -
	// see _shifter's own comment), and every tick copies each shared
	// node's position across from positionSource rather than computing
	// its own via spring physics. start() must not be called on a mirror
	// instance - positionSource's own start() already drives it.
	//
	// positionOffset: only meaningful alongside positionSource - the
	// constant screen-space delta between this diagram's own eventual
	// setPosition() target and positionSource's own (e.g. 2x the
	// side-by-side offset between two diagrams placed left/right of a
	// shared centre), added to every position syncPositionsFrom() copies
	// across. Without this, a mirror's nodes would land exactly on top
	// of positionSource's own (see syncPositionsFrom()'s own comment for
	// why) rather than offset the way the two diagrams' own one-time
	// setPosition() calls intended.
	//
	// targetRadius: how far (screen units) the furthest atom from the
	// group's own centroid should land - see _displayScale's own comment.
	// A mirror instance must always be given the same value its own
	// positionSource was, since a mismatch here would make
	// syncPositionsFrom()'s copy a differently-scaled (distorted) mirror
	// of positionSource's own layout rather than a true one - see
	// ViewCorrelations::showHydrogenBondDiagram()'s own offset comment
	// for how it separately compensates for a diagram whose real
	// geometry happens to need more or less screen room.
	//
	// hoverCallback: called with a hovered atom/bond's own Probe::desc()
	// on hover, and with an empty string on unhover - lets a caller show
	// that identity somewhere of its own choosing (e.g. one shared label
	// positioned below both diagrams - see makeAtoms()'s own comment for
	// why per-atom Box::addAltTag() doesn't work here) rather than each
	// atom trying to show its own floating tag. Never called at all if
	// left default (empty std::function).
	HBondDiagram(const OpSet<ProbeTypePair> &nodes, const glm::mat4x4 &model,
	            const CertainStates *states = nullptr,
	            const std::vector<float> &stateWeights = {},
	            HBondDiagram *positionSource = nullptr,
	            const glm::vec3 &positionOffset = {},
	            float targetRadius = 0.5f,
	            std::function<void(const std::string &)> hoverCallback = {});
	~HBondDiagram();

	// runs this diagram's own PositionShifter synchronously (see
	// PositionShifter::settle() itself) rather than starting the usual
	// background thread, so a caller can measure a genuinely relaxed
	// (repulsion/spring-settled) layout - see measuredRadius() - before
	// ever deciding this diagram's own screen placement from it. Must be
	// called (if at all) before start(), on a caller that owns its own
	// physics (_ownsShifter) - a no-op mirror instance has nothing of its
	// own to settle, since its positions are entirely copied from
	// positionSource instead (see the constructor's own comment).
	void settle(int iterations);

	// how far (screen units) the furthest currently-displayed atom
	// actually sits from this diagram's own current centroid - unlike
	// _targetRadius (the seed value makeAtoms() aimed for before any
	// physics ran at all), this reflects whatever PositionShifter's
	// repulsion has actually done since, e.g. after settle() above. 0 if
	// there is nothing displayed yet.
	float measuredRadius() const;

	// starts the physics thread - deliberately not done by the
	// constructor, mirroring ClusterPlot::start() (see its own comment):
	// callers position the whole diagram afterwards via setPosition(),
	// before the physics thread starts writing to the same Renderables.
	// Must not be called on a mirror instance (positionSource given to
	// the constructor) - see its own comment.
	void start();

private:
	// atom-typed nodes: one Text label + one PositionShifter element per
	// atom, keyed by the underlying Probe* (matches _reach's keys, and
	// the getter/setter closures PositionShifter's gradient() calls).
	void makeAtoms();

	// bond-typed nodes whose both endpoints are also atom-typed nodes
	// this diagram is displaying (see makeAtoms()) - anything else (a
	// bond reaching outside this subnetwork's own node set) is skipped;
	// there is nothing on screen to connect it to. Also draws the
	// covalent skeleton between these same atoms - _nodes (ptps())
	// deliberately never includes covalent bonds (ExhaustiveSearch skips
	// is_covalent() probes entirely), so those are found separately by
	// walking each displayed atom's own Probe::others() instead.
	void makeBonds();

	// shared by both passes in makeBonds() - draws one bond line between
	// probe's two endpoints, if both are already-displayed atoms (see
	// makeAtoms()); a no-op otherwise.
	void makeBondLine(Probe *probe);

	// copies each shared node's underlying sim-space position
	// (PositionShifter's own internal storage, pre-_displayScale/
	// to_screen - see makeAtoms()) from source into this diagram's own
	// matching nodes, then re-renders. Mirror instances (positionSource
	// given to the constructor) register this as a Tidy job on the
	// shared PositionShifter, run every tick alongside source's own
	// atoms' physics-driven updates - see the constructor's own comment.
	// A no-op for any Probe* this diagram does not itself display.
	void syncPositionsFrom(const HBondDiagram &source);

	// fills _alphaOverride/_bondOverlay from states/stateWeights (see the
	// constructor's own comment) - a no-op (both maps left empty) if
	// states is null, so makeAtoms()/makeBondLine() fall back to the
	// plain live-snapshot probe->alpha()/probe->display() exactly as
	// before. Two passes: ExistenceType ptps() first (one per atom, the
	// fraction of stateWeights-weighted states where that atom is
	// Present), then BondType ptps() (one per bond, filling _bondOverlay
	// with two independently-faded layers rather than picking one
	// "majority" icon - see its own body for why only weak/strong, not
	// also a separate broken/lone-pair/absent layer, are distinguishable
	// at all) - the second pass also fills in alpha for each bond's own
	// bridging-hydrogen endpoint (never its own ExistenceType ptp - see
	// makeAtoms()'s own comment on why), but only if the first pass
	// hasn't already claimed that Probe*, which is why the two passes run
	// in this order and not combined into one.
	void computeStateAverages(const CertainStates *states,
	                          const std::vector<float> &stateWeights);

	OpSet<ProbeTypePair> _nodes;

	std::map<Probe *, Text *> _atoms;

	// see the constructor's own comment - empty (never called) unless a
	// caller passed one in.
	std::function<void(const std::string &)> _hoverCallback;

	// see computeStateAverages() - empty (falls back to probe->alpha()/
	// probe->display()) unless this instance was constructed with a
	// CertainStates/stateWeights pair.
	std::map<Probe *, float> _alphaOverride;

	// a bond probe's state-weighted average, expressed as independently-
	// faded overlay layers (icon name, alpha) rather than one "majority"
	// icon - see computeStateAverages()'s own comment for why only
	// weak_bond/strong_bond (never a separate broken/lone-pair/absent
	// layer too - certainValueAsInt() collapses all three into the same
	// recorded value, so CertainStates has no way to tell them apart in
	// hindsight; a bond that is mostly absent is adequately shown by both
	// layers below simply fading out together). Drawn by makeBondLine()
	// as several overlapping BondRods sharing one position; empty (falls
	// back to one BondRod via probe->display()) unless this instance was
	// constructed with a CertainStates/stateWeights pair.
	std::map<Probe *, std::vector<std::pair<std::string, float>>> _bondOverlay;

	// bridging-hydrogen label text only ("H" vs " ", majority-bonded vs
	// not - see computeStateAverages()'s own comment) - every other atom's
	// display() text (an element symbol) does not vary with existence
	// certainty at all, so nothing else ever needs an entry here.
	std::map<Probe *, std::string> _textOverride;

	// converts between screen space (what each Text label actually
	// renders at - this Scene's shaders treat vertex positions as literal
	// final NDC coordinates, no camera/projection to bring real-world
	// coordinates back into [-1,1]) and "real" space (Probe::_init's own
	// Angstrom-scale units) - picked in makeAtoms() so the atom furthest
	// from the group's own centroid maps to _targetRadius screen units.
	// Same role as ClusterPlot::_displayScale (see its own comment) - the
	// default PositionShifter targetFn compares its target (real _init
	// distances, unscaled) against actual (from the getter/setter below,
	// converted back to real-ish units by dividing this out) - a shared
	// constant centroid offset cancels out of both sides of that
	// comparison, so only the scale has to match, not any absolute origin.
	float _displayScale = 1.f;

	// see the constructor's own comment - 0.5 unless overridden.
	float _targetRadius = 0.5f;

	// per-atom "reach" (1 and 2 covalent bonds away) - same scheme and
	// same motivation as ProtonNetworkView's own _reach (see its
	// comment): a real distance-matching spring only makes sense between
	// atoms actually related through the bond graph, not every pair.
	// Keyed by Probe* (the reference passed to PositionShifter::
	// addPosition() below), restricted to Probes actually in _atoms -
	// reach reaching outside this diagram's own subset is simply never
	// recorded, since there is no Renderable for it to weight against.
	std::map<void *, OpSet<void *>> _reach;

	// owned (and deleted in the destructor) unless this instance is a
	// mirror (positionSource given to the constructor), in which case
	// this points to positionSource's own _shifter instead - see the
	// constructor's own comment for why one shared physics thread is
	// enough for both diagrams. _ownsShifter records which case this is.
	PositionShifter *_shifter = nullptr;
	bool _ownsShifter = true;

	// see the constructor's own comment - only meaningful for a mirror
	// instance, added to every position syncPositionsFrom() copies across.
	glm::vec3 _positionOffset{};

	// non-owning - only set (to positionSource) for a mirror instance,
	// purely so a mirror's own atoms can be dragged independently (see
	// makeAtoms()'s own comment) - the drag callback needs to recompute
	// "wherever syncPositionsFrom() would otherwise have placed this atom
	// right now" the same way syncPositionsFrom() itself does, which
	// means reading positionSource's own _shifter directly.
	HBondDiagram *_positionSource = nullptr;

	// mirror-only (see makeAtoms()'s own comment) - a per-atom screen-
	// space delta that syncPositionsFrom() adds on top of whatever
	// position it would otherwise compute, letting a mirror's own node be
	// dragged to a different spot than its counterpart in positionSource
	// while still tracking that counterpart's own ongoing physics-driven
	// motion, rather than freezing in place the moment it's dragged.
	// Empty (no entry) for any atom never manually moved - the common
	// case, and the only one that applies at all to a non-mirror instance.
	std::map<Probe *, glm::vec3> _manualOffset;

	// every Tidy job this class registers (makeAtoms()'s per-atom
	// forceRender, makeBondLine()'s per-bond fixVertices, syncPositionsFrom
	// for a mirror) checks this first before touching anything this
	// instance owns. Necessary because deleteTemps() queues both
	// diagrams' deletion into the same pointer-ordered std::set (Window::
	// _deleteRenderables - see its own comment), not in a guaranteed safe
	// order, while the (possibly shared) physics thread keeps running
	// right up until whichever instance owns it actually stops it in its
	// own destructor - so a Tidy job belonging to an already-destroyed
	// instance still getting invoked is a real possibility, not just a
	// theoretical one. Flipped false as the very first thing this
	// instance's own destructor does; captured by value (the shared_ptr
	// itself, which outlives the object it flags, not just the bool) by
	// every Tidy job lambda this class registers.
	std::shared_ptr<std::atomic<bool>> _alive =
	std::make_shared<std::atomic<bool>>(true);
};

#endif
