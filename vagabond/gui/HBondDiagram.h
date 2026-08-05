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
#include <map>

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
	HBondDiagram(const OpSet<ProbeTypePair> &nodes, const glm::mat4x4 &model,
	            const CertainStates *states = nullptr,
	            const std::vector<float> &stateWeights = {});
	~HBondDiagram();

	// starts the physics thread - deliberately not done by the
	// constructor, mirroring ClusterPlot::start() (see its own comment):
	// callers position the whole diagram afterwards via setPosition(),
	// before the physics thread starts writing to the same Renderables.
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

	// fills _alphaOverride/_iconOverride from states/stateWeights (see the
	// constructor's own comment) - a no-op (both maps left empty) if
	// states is null, so makeAtoms()/makeBondLine() fall back to the
	// plain live-snapshot probe->alpha()/probe->display() exactly as
	// before. Two passes: ExistenceType ptps() first (one per atom, the
	// fraction of stateWeights-weighted states where that atom is
	// Present), then BondType ptps() (one per bond, the fraction where it
	// is genuinely bonded (Weak/Acceptor or Strong/Donor) rather than
	// Absent/Broken/LonePair, plus a majority-role icon - see its own
	// body) - the second pass also fills in alpha for each bond's own
	// bridging-hydrogen endpoint (never its own ExistenceType ptp - see
	// makeAtoms()'s own comment on why), but only if the first pass
	// hasn't already claimed that Probe*, which is why the two passes run
	// in this order and not combined into one.
	void computeStateAverages(const CertainStates *states,
	                          const std::vector<float> &stateWeights);

	OpSet<ProbeTypePair> _nodes;

	std::map<Probe *, Text *> _atoms;

	// see computeStateAverages() - empty (falls back to probe->alpha()/
	// probe->display()) unless this instance was constructed with a
	// CertainStates/stateWeights pair.
	std::map<Probe *, float> _alphaOverride;
	std::map<Probe *, std::string> _iconOverride;

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
	// from the group's own centroid maps to about half a screen unit.
	// Same role as ClusterPlot::_displayScale (see its own comment) - the
	// default PositionShifter targetFn compares its target (real _init
	// distances, unscaled) against actual (from the getter/setter below,
	// converted back to real-ish units by dividing this out) - a shared
	// constant centroid offset cancels out of both sides of that
	// comparison, so only the scale has to match, not any absolute origin.
	float _displayScale = 1.f;

	// per-atom "reach" (1 and 2 covalent bonds away) - same scheme and
	// same motivation as ProtonNetworkView's own _reach (see its
	// comment): a real distance-matching spring only makes sense between
	// atoms actually related through the bond graph, not every pair.
	// Keyed by Probe* (the reference passed to PositionShifter::
	// addPosition() below), restricted to Probes actually in _atoms -
	// reach reaching outside this diagram's own subset is simply never
	// recorded, since there is no Renderable for it to weight against.
	std::map<void *, OpSet<void *>> _reach;

	PositionShifter *_shifter = nullptr;
};

#endif
